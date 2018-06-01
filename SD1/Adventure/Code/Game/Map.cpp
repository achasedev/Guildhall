/************************************************************************/
/* File: Map.cpp
/* Author: Andrew Chase
/* Date: November 4th, 2017
/* Bugs: None
/* Description: Implementation of the Map class
/************************************************************************/
#include "Game/Map.hpp"
#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"
#include "Game/MapDefinition.hpp"
#include "Game/Actor.hpp"
#include "Game/ActorDefinition.hpp"
#include "Game/Projectile.hpp"
#include "Game/ProjectileDefinition.hpp"
#include "Game/Player.hpp"
#include "Game/Portal.hpp"
#include "Game/Adventure.hpp"
#include "Game/Item.hpp"
#include "Game/ItemDefinition.hpp"
#include "Game/Camera.hpp"
#include "Engine/Math/IntRange.hpp"
#include "Engine/Math/Disc2.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Renderer/Renderer.hpp"


// Draw bounds for the pickup text
const AABB2 Map::PICKUP_DRAW_BOX = AABB2(Vector2(0.f, 7.0f), Vector2(6.0f, 9.f));


//-----------------------------------------------------------------------------------------------
// Constructs the map tiles and applies the generators
//
Map::Map(const std::string& name, const MapDefinition* definition, Adventure* adventure)
	: m_name(name)
	, m_definition(definition)
	, m_theAdventure(adventure)
{
	// Setup the game dimensions
	m_dimensions.x = m_definition->GetRandomWidthInRange();
	m_dimensions.y = m_definition->GetRandomHeightInRange();
	m_tiles.resize(m_dimensions.x * m_dimensions.y);

	// Initialize the tiles to the default tile of this definition
	const TileDefinition* defaultTile = m_definition->GetDefaultTile();

	for (int rowIndex = 0; rowIndex < m_dimensions.y; ++rowIndex)
	{
		for (int colIndex = 0; colIndex < m_dimensions.x; ++colIndex)
		{	
			IntVector2 currCoords = IntVector2(colIndex, rowIndex);
			int index = (currCoords.y * m_dimensions.x) + currCoords.x;
			Tile& currTile = m_tiles.at(index);
			currTile = Tile(currCoords, defaultTile);		
		}
	}

	// Run the generators on this map
	std::vector<MapGenStep*> generators = m_definition->GetGenerators();
	for (int genIndex = 0; genIndex < static_cast<int>(generators.size()); genIndex++)
	{
		generators[genIndex]->Run(*this);
	}
}


//-----------------------------------------------------------------------------------------------
// Destructor
//
Map::~Map()
{
	m_isBeingDeleted = true;
	for (int entityIndex = 0; entityIndex >=  static_cast<int>(m_entities.size()); entityIndex++)
	{
		delete m_entities[entityIndex];
		// No need to erase, since the vectors will all be destroyed
	}
}


//-----------------------------------------------------------------------------------------------
// Returns the dimensions of this map instance
//
IntVector2 Map::GetDimensions() const
{
	return m_dimensions;
}


//-----------------------------------------------------------------------------------------------
// Returns the tile that contains the given position
//
Tile* Map::GetTileFromPosition(const Vector2& position)
{
	IntVector2 tileCoords = IntVector2(static_cast<int>(position.x), static_cast<int>(position.y));
	return GetTileFromCoords(tileCoords);
}


//-----------------------------------------------------------------------------------------------
// Returns the tile that is at the given coordinates
//
Tile* Map::GetTileFromCoords(const IntVector2& tileCoords)
{
	// Safety check - index should be in bounds
	if (!AreCoordsValid(tileCoords))
	{
		return nullptr;
	}

	int index = (tileCoords.y * m_dimensions.x) + tileCoords.x;
	return &m_tiles.at(index);
}


//-----------------------------------------------------------------------------------------------
// Returns the tile that is at the given coordinates, constant
//
const Tile* Map::GetTileFromCoordsConst(const IntVector2& tileCoords) const
{
	// Safety check - index should be in bounds
	if (!AreCoordsValid(tileCoords))
	{
		return nullptr;
	}

	int index = (tileCoords.y * m_dimensions.x) + tileCoords.x;
	return &m_tiles.at(index);
}


//-----------------------------------------------------------------------------------------------
// Returns the tile that is at the given index
//
Tile* Map::GetTileFromIndex(int index)
{
	// Ensure index is in range
	int numTiles = static_cast<int>(m_tiles.size());
	if (index >= 0 && index < numTiles)
	{
		return &m_tiles.at(index);
	}

	return nullptr;
}


//-----------------------------------------------------------------------------------------------
// Finds a random tile of the given type on this map and returns it
//
Tile* Map::GetRandomTileOfType(const TileDefinition* tileType)
{
	// Return any random tile if no type was specified
	if (tileType == nullptr)
	{
		int randomIndex = GetRandomIntLessThan(static_cast<int>(m_tiles.size()));
		return &m_tiles[randomIndex];
	}

	// Type not null, so find all tiles of that type and return a random one
	std::vector<int> tileIndicesOfType;

	for (int tileIndex = 0; tileIndex < static_cast<int>(m_tiles.size()); tileIndex++)
	{
		Tile& currTile = m_tiles[tileIndex];

		if (currTile.GetDefinition() == tileType)
		{
			tileIndicesOfType.push_back(tileIndex);
		}
	}

	// Ensure a tile of this type exists
	GUARANTEE_OR_DIE(tileIndicesOfType.size() > 0, Stringf("Error: Map::GetTileFromType could not find any tiles of type \"%s\" in map name \"%s\"", tileType->GetName().c_str(), m_name.c_str()));

	// Get a random index from our vector of indices and return
	int randomTypeIndex = GetRandomIntLessThan(static_cast<int>(tileIndicesOfType.size()));
	int tileIndex		= tileIndicesOfType[randomTypeIndex];
	
	return &m_tiles.at(tileIndex);
}


//-----------------------------------------------------------------------------------------------
// Returns a reference to a tile that this actor can walk on
//
Tile* Map::GetTileEntityCanEnter(Entity* entity)
{
	std::vector<int> tilesEntityCanEnter;

	// Iterate across all tiles, collecting those this entity can enter
	for (int tileIndex = 0; tileIndex < static_cast<int>(m_tiles.size()); tileIndex++)
	{
		Tile& currTile = m_tiles[tileIndex];

		if (EntityCanEnterTile(entity, &currTile))
		{
			tilesEntityCanEnter.push_back(tileIndex);
		}
	}

	// Ensure a tile of this type exists
	GUARANTEE_OR_DIE(tilesEntityCanEnter.size() > 0, Stringf("Error: Map::GetTileEntityCanEnter could not find any tiles that entity \"%s\" can enter in map \"%s\"""", entity->GetName().c_str(), m_name.c_str()));

	// Get a random index from our vector of indices and return
	int randomTypeIndex = GetRandomIntLessThan(static_cast<int>(tilesEntityCanEnter.size()));
	int tileIndex		= tilesEntityCanEnter[randomTypeIndex];

	return &m_tiles.at(tileIndex);
}


//-----------------------------------------------------------------------------------------------
// Returns true if the coords are in bounds of the map, false otherwise
//
bool Map::AreCoordsValid(const IntVector2& coords) const
{
	bool withinHorizontalBounds		= (coords.x >= 0 && coords.x < m_dimensions.x);
	bool withinVerticalBounds		= (coords.y >= 0 && coords.y < m_dimensions.y);

	return (withinHorizontalBounds && withinVerticalBounds);
}


//-----------------------------------------------------------------------------------------------
// Updates the map and all entities on it
//
void Map::Update(float deltaTime)
{
	ProcessMapInput();						// Update map based on map input
	UpdateEntities(deltaTime);				// Updates all Entities' state
	CheckForAndCorrectActorCollisions();	// Checks for collisions between actors, and fixes them	
	CheckForAndCorrectTileCollisions();		// Checks for collisions between entities and tiles, and fixes them
	CheckProjectilesAgainstActors();		// Projectile collision with Actors
	CheckActorsAgainstPortals();			// Checks to see if any Actors interacted with a portal this frame
	CheckForItemPickup();					// Checks if the player is on an item, for pickup and equip
	DeleteMarkedEntities();					// Deletes all entities marked for delete this frame
}


//-----------------------------------------------------------------------------------------------
// Draws all tiles and entities on this map
//
void Map::Render() const
{
	// Render the tiles
	RenderTiles();

	// Renders the entities
	for (Entity* curr : m_entities)
	{
		// Only draw entities that are in view
		if (g_theCamera->IsInCameraView(curr->GetWorldDrawBounds()))
		{
			curr->Render();
		}
	}

	// Draw the pickup text if we're on an item
	if (m_itemPlayerIsOn != nullptr)
	{
		DrawPickupText();
	}
}


//-----------------------------------------------------------------------------------------------
// Spawns an actor on this map
//
void Map::SpawnActor(const Vector2& position, float orientation, const ActorDefinition* definition, const std::string& name)
{
	// Add a random offset to ensure actors don't spawn on top of each other
	Vector2 randomOffset = Vector2::GetRandomVector(0.05f);
	Actor* newActor = new Actor(position + randomOffset, orientation, definition, this, name);
	AddActorToMap(newActor);
}


//-----------------------------------------------------------------------------------------------
// Spawns a projectile on this map
//
Projectile* Map::SpawnProjectile(const Vector2& position, float orientation, const ProjectileDefinition* definition, const std::string& name, const std::string& faction)
{
	Projectile* newProjectile = new Projectile(position, orientation, definition, this, name, faction);
	AddProjectileToMap(newProjectile);

	return newProjectile;
}


//-----------------------------------------------------------------------------------------------
// Spawns a portal on this map
//
void Map::SpawnPortal(const Vector2& position, float orientation, const PortalDefinition* definition, const std::string& name, Map* destinationMap, const Vector2& destinationPosition)
{
	Portal* newPortal = new Portal(position, orientation, definition, this, name, destinationMap, destinationPosition);
	AddPortalToMap(newPortal);
}


//-----------------------------------------------------------------------------------------------
// Spawns an item on this map
//
void Map::SpawnItem(const Vector2& position, float orientation, const ItemDefinition* definition, const std::string& name)
{
	Item* newItem = new Item(position, orientation, definition, this, name);
	AddItemToMap(newItem);
}


//-----------------------------------------------------------------------------------------------
// Removes the actor from this map's entity lists - does NOT delete the actor
//
void Map::RemoveActorFromMap(Actor* actorToRemove)
{
	// Remove from the actor list
	for (int actorIndex = 0; actorIndex < static_cast<int>(m_actors.size()); actorIndex++)
	{
		Actor* currActor = m_actors[actorIndex];
		if (currActor == actorToRemove)
		{
			m_actors.erase(m_actors.begin() + actorIndex);
			break;
		}
	}

	// Remove from the full entity list
	for (int entityIndex = 0; entityIndex < static_cast<int>(m_entities.size()); entityIndex++)
	{
		Entity* currEntity = m_entities[entityIndex];
		if (currEntity == actorToRemove)
		{
			m_entities.erase(m_entities.begin() + entityIndex);
			break;
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Removes the actor from this map's entity lists - does NOT delete the actor
//
void Map::RemoveProjectileFromMap(Projectile* projectileToRemove)
{
	// Remove from the actor list
	for (int projectileIndex = 0; projectileIndex < static_cast<int>(m_projectiles.size()); projectileIndex++)
	{
		Projectile* currProjectile = m_projectiles[projectileIndex];
		if (currProjectile == projectileToRemove)
		{
			m_projectiles.erase(m_projectiles.begin() + projectileIndex);
			break;
		}
	}

	// Remove from the full entity list
	for (int entityIndex = 0; entityIndex < static_cast<int>(m_entities.size()); entityIndex++)
	{
		Entity* currEntity = m_entities[entityIndex];
		if (currEntity == projectileToRemove)
		{
			m_entities.erase(m_entities.begin() + entityIndex);
			break;
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Removes the item from this map's entity lists - does NOT delete the item
//
void Map::RemoveItemFromMap(Item* itemToRemove)
{
	// Remove from the item list
	for (int itemIndex = 0; itemIndex < static_cast<int>(m_items.size()); itemIndex++)
	{
		Item* currItem = m_items[itemIndex];
		if (currItem == itemToRemove)
		{
			m_items.erase(m_items.begin() + itemIndex);
			break;
		}
	}

	// Remove from the full entity list
	for (int entityIndex = 0; entityIndex < static_cast<int>(m_entities.size()); entityIndex++)
	{
		Entity* currEntity = m_entities[entityIndex];
		if (currEntity == itemToRemove)
		{
			m_entities.erase(m_entities.begin() + entityIndex);
			break;
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Adds the actor to the current map - used for the player and for portals
//
void Map::AddActorToMap(Actor* actorToAdd)
{
	m_entities.push_back(actorToAdd);
	m_actors.push_back(actorToAdd);
}


//-----------------------------------------------------------------------------------------------
// Adds the given item to the map
//
void Map::AddItemToMap(Item* itemToAdd)
{
	m_entities.push_back(itemToAdd);
	m_items.push_back(itemToAdd);

	// Ensure the item is using the correct map animation
	itemToAdd->StartAnimation("WorldSprite");
	itemToAdd->SetActorHoldingThis(nullptr);
}


//-----------------------------------------------------------------------------------------------
// Adds the projectile to the map
//
void Map::AddProjectileToMap(Projectile* projectileToAdd)
{
	m_entities.push_back(projectileToAdd);
	m_projectiles.push_back(projectileToAdd);

	// Ensure the item is using the correct map animation
	projectileToAdd->StartAnimation("WorldSprite");
}


//-----------------------------------------------------------------------------------------------
// Adds the portal to the map
//
void Map::AddPortalToMap(Portal* portalToAdd)
{
	m_entities.push_back(portalToAdd);
	m_portals.push_back(portalToAdd);

	// Ensure the item is using the correct map animation
	portalToAdd->StartAnimation("WorldSprite");
}


//-----------------------------------------------------------------------------------------------
// Sets all portals to not be ready to prevent endless teleports
//
void Map::DisableAllPortals()
{
	for (int portalIndex = 0; portalIndex < static_cast<int>(m_portals.size()); portalIndex++)
	{
		m_portals[portalIndex]->SetIsReady(false);
	}
}


//-----------------------------------------------------------------------------------------------
// Returns the nearest hostile target in range and line of sight to attackingEntity
//
Actor* Map::FindBestHostileActor(Actor* attackingActor) const
{
	Actor* bestTarget = nullptr;
	float bestDistance = -1.f;

	for (int actorIndex = 0; actorIndex < static_cast<int>(m_actors.size()); actorIndex++)
	{
		Actor* currTargetActor = m_actors[actorIndex];

		// Ensure the target is alive and is of a hostile faction
		if (currTargetActor->IsMarkedForDeletion() || AreSameFaction(currTargetActor, attackingActor)) { continue; }

		if (HasLineOfSight(attackingActor->GetPosition(), currTargetActor->GetPosition(), attackingActor->GetViewDistance()))
		{
			float currentDistanceBetween = (attackingActor->GetPosition() - currTargetActor->GetPosition()).GetLength();
			if (bestTarget == nullptr || (currentDistanceBetween < bestDistance))
			{
				bestTarget = currTargetActor;
				bestDistance = currentDistanceBetween;
			}
		}
	}

	return bestTarget;
}


//-----------------------------------------------------------------------------------------------
// Returns true if the given entities are of the same faction
//
bool Map::AreSameFaction(Entity* firstActor, Entity* secondActor) const
{
	std::string firstFaction = firstActor->GetFaction();
	std::string secondFaction = secondActor->GetFaction();

	return (firstFaction.compare(secondFaction) == 0);
}


//-----------------------------------------------------------------------------------------------
// Performs a raycast from startPosition in direction out maxDistance, and returns a result as soon
// as a collision is found
//
RaycastResult Map::Raycast(const Vector2& startPosition, const Vector2& normalizedDirection, float maxDistance) const
{
	int numSteps		= static_cast<int>(maxDistance * RAYCAST_STEPS_PER_TILE);
	Vector2 stepSize	= (normalizedDirection / static_cast<float>(RAYCAST_STEPS_PER_TILE));

	IntVector2 previousTileCoords = IntVector2(static_cast<int>(startPosition.x), static_cast<int>(startPosition.y));

	// Iterate the required number of steps to cover maxDistance
	for (int stepNumber = 0; stepNumber <= numSteps; ++stepNumber)
	{
		// Recalculate the step each time to prevent floating point error build up
		Vector2 currPosition = startPosition + (stepSize * static_cast<float>(stepNumber));

		IntVector2 currTileCoords = IntVector2(static_cast<int>(currPosition.x), static_cast<int>(currPosition.y));

		// If we haven't changed tiles, then we know there is no new information, so continue
		if (currTileCoords == previousTileCoords) { continue; }

		// New tile, check if it allows sight
		const Tile* currTile = GetTileFromCoordsConst(currTileCoords);

		if (currTile == nullptr || !currTile->GetDefinition()->AllowsSight())
		{
			// Make a RaycastResult for this impact
			RaycastResult impactResult;
			impactResult.m_didImpact		= true;
			impactResult.m_impactPosition	= currPosition;
			impactResult.m_impactTileCoords	= currTileCoords;
			impactResult.m_impactDistance	= (static_cast<float>(stepNumber) * stepSize).GetLength();
			impactResult.m_impactFraction	= (impactResult.m_impactDistance / maxDistance);
			impactResult.m_impactNormal		= Vector2(currTileCoords - previousTileCoords);
			return impactResult;
		}

		// Otherwise update our previous tile coordinates
		previousTileCoords = currTileCoords;
	}

	// Full distance traveled and there was no impact, so return a RaycastResult for no impact
	RaycastResult noImpactResult;
	noImpactResult.m_didImpact			= false;																													// No impact
	noImpactResult.m_impactPosition		= startPosition + (normalizedDirection * maxDistance);																		// The end point of our ray cast
	noImpactResult.m_impactTileCoords	= IntVector2(static_cast<int>(noImpactResult.m_impactPosition.x), static_cast<int>(noImpactResult.m_impactPosition.y));		// Tile that contains the end point
	noImpactResult.m_impactDistance		= maxDistance;																												// Ray went the full distance
	noImpactResult.m_impactFraction		= 1.f;																														// Ray went the full distance
	noImpactResult.m_impactNormal		= Vector2::ZERO;																											// There was no impact, so no impact normal

	return noImpactResult;
}


//-----------------------------------------------------------------------------------------------
// Returns true if there is a clear line of sight between startPos and endPos, and if they are within
// viewDistance from each other
//
bool Map::HasLineOfSight(const Vector2& startPos, const Vector2& endPos, float viewDistance) const
{
	Vector2 directionToCast = (endPos - startPos);

	// In case the positions are equal
	if (directionToCast == Vector2::ZERO)
	{
		return true;
	}

	float distanceBetween = directionToCast.NormalizeAndGetLength();

	// Immediately return if the two points are farther apart than viewDistance
	if (distanceBetween > viewDistance) 
	{ 
		return false; 
	}

	RaycastResult raycastResult = Raycast(startPos, directionToCast, distanceBetween);

	return (!raycastResult.m_didImpact);
}


//-----------------------------------------------------------------------------------------------
// Returns whether the map is currently being deleted
//
bool Map::IsBeingDeleted() const
{
	return m_isBeingDeleted;
}


//-----------------------------------------------------------------------------------------------
// Parses input that changes map state
//
void Map::ProcessMapInput()
{
	if (m_itemPlayerIsOn != nullptr)
	{
		XboxController& controller = g_theInputSystem->GetController(0);
		if (controller.WasButtonJustPressed(XBOX_BUTTON_X))
		{
			g_thePlayer->AddItemToInventory(m_itemPlayerIsOn);
			RemoveItemFromMap(m_itemPlayerIsOn);
			m_itemPlayerIsOn = nullptr;
		}
		else if (controller.WasButtonJustPressed(XBOX_BUTTON_Y))
		{
			g_thePlayer->EquipItem(m_itemPlayerIsOn);
			RemoveItemFromMap(m_itemPlayerIsOn);
			m_itemPlayerIsOn = nullptr;
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Calls Update on all entities in the map
//
void Map::UpdateEntities(float deltaTime)
{
	for (int entityIndex = 0; entityIndex < static_cast<int>(m_entities.size()); entityIndex++)
	{
		Entity* currEntity = m_entities[entityIndex];
		if (IsEntityAlive(currEntity))
		{
			currEntity->Update(deltaTime);
		}
	}

	// Run a single pass through the entity list to sort by position y-value - for draw order
	SortEntitiesByPosition();
}


//-----------------------------------------------------------------------------------------------
// Checks if any two actors are colliding with each other, and corrects them if so
//
void Map::CheckForAndCorrectActorCollisions()
{
	// Getting the first actor to check
	for (int firstIndex = 0; firstIndex < static_cast<int>(m_actors.size()) - 1; firstIndex++)
	{
		Actor* firstActor = m_actors[firstIndex];
		if (!firstActor->IsUsingPhysics()) { continue; }

		// Iterate across all actors after firstActor to compare
		for (int secondIndex = firstIndex + 1; secondIndex < static_cast<int>(m_actors.size()); secondIndex++)
		{
			Actor* secondActor = m_actors[secondIndex];
			if (!secondActor->IsUsingPhysics()) { continue; }

			// Check to see if these two entities are overlapping
			if (DoDiscsOverlap(firstActor->GetPosition(), firstActor->GetInnerRadius(), secondActor->GetPosition(), secondActor->GetInnerRadius()))
			{
				CorrectActorCollision(firstActor, secondActor);
			}
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Pushes the two given entities out of each other evenly
//
void Map::CorrectActorCollision(Actor* firstActor, Actor* secondActor)
{
	float sumOfRadii				= (firstActor->GetInnerRadius() + secondActor->GetInnerRadius());
	Vector2 displacementToSecond	= (secondActor->GetPosition() - firstActor->GetPosition());
	float distance					= displacementToSecond.NormalizeAndGetLength();

	// Check for if the two entities are on top of each other exactly, and if so just push them out in a random direction
	if (distance == 0.f)
	{
		ERROR_RECOVERABLE(Stringf("Error: Two actors were directly on top of eachother: \"%s\" and \"%s\"", firstActor->GetName().c_str(), secondActor->GetName().c_str()));
		displacementToSecond = Vector2(GetRandomFloatZeroToOne(), GetRandomFloatZeroToOne()).GetNormalized();
	}

	// Total distance for the two entities to be separated by
	float totalCorrectiveMagnitude	= (sumOfRadii - distance);

	Vector2 firstActorCorrection	= (-0.5f * displacementToSecond * totalCorrectiveMagnitude);
	Vector2 secondActorCorrection	= (0.5f * displacementToSecond * totalCorrectiveMagnitude);

	// Adjust both entities' position to be correct
	firstActor->SetPosition(firstActor->GetPosition() + firstActorCorrection);
	secondActor->SetPosition(secondActor->GetPosition() + secondActorCorrection);
}


//-----------------------------------------------------------------------------------------------
// Runs through the entity list once and swaps any neighbor entities such that the list is in
// order of decreasing Y value
//
void Map::SortEntitiesByPosition()
{
	for (int entityIndex = 0; entityIndex < static_cast<int>(m_entities.size()) - 1; entityIndex++)
	{
		// If the first index's y value is less than the successor, swap the two
		if (m_entities[entityIndex]->GetPosition().y < m_entities[entityIndex + 1]->GetPosition().y)
		{
			Entity* temp = m_entities[entityIndex];
			m_entities[entityIndex] = m_entities[entityIndex + 1];
			m_entities[entityIndex + 1] = temp;
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Checks all entities for tile collisions
//
void Map::CheckForAndCorrectTileCollisions()
{
	// Check actors for collision, and correct them if so
	for (Actor* curr : m_actors)
	{
		if (IsEntityAlive(curr) && curr->IsUsingPhysics())
		{
			CheckEntityForAdjacentTileCollisions(curr, false);
		}
	}

	// Check projectiles for collision, and destroy them if so
	for (Projectile* curr : m_projectiles)
	{
		if (IsEntityAlive(curr) && curr->IsUsingPhysics())
		{
			CheckEntityForAdjacentTileCollisions(curr, true);
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Checks if any projectiles collide with hostile actors, and destroys/deals damage accordingly
//
void Map::CheckProjectilesAgainstActors()
{
	for (int projectileIndex = 0; projectileIndex < static_cast<int>(m_projectiles.size()); projectileIndex++)
	{
		Projectile* currProjectile = m_projectiles[projectileIndex];
		if (!IsEntityAlive(currProjectile)) { continue; }

		for (int actorIndex = 0; actorIndex < static_cast<int>(m_actors.size()); actorIndex++)
		{
			Actor* currActor = m_actors[actorIndex];
			if (!IsEntityAlive(currActor) || AreSameFaction(currProjectile, currActor) || !currActor->IsUsingPhysics()) { continue; }

			// Check for collision
			if (DoDiscsOverlap(currProjectile->GetPosition(), currProjectile->GetInnerRadius(), currActor->GetPosition(), currActor->GetInnerRadius()))
			{
				// Damage the entities
				currProjectile->SetMarkedForDeath(true);	// Kill the bullet
				currActor->DoDamageCheck(currProjectile);	// Damage the actor
			}
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Checks if an actor is touching an active portal, and if so, teleports them
// If a portal is not ready and no actors were touching it, it is marked as ready
//
void Map::CheckActorsAgainstPortals()
{
	for (int portalIndex = 0; portalIndex < static_cast<int>(m_portals.size()); portalIndex++)
	{
		Portal* currPortal = m_portals[portalIndex];
		bool portalReady = currPortal->IsReady(); 
		bool didEntityOverlapPortal = false;

		// Iterate across all Actors to check for collision
		for (int actorIndex = 0; actorIndex < static_cast<int>(m_actors.size()); actorIndex++)
		{

			Actor* currActor = m_actors[actorIndex];
			if (!IsEntityAlive(currActor)) { continue; }

			// If the portal is ready, check to teleport an actor
			if (portalReady)
			{
				if (DoesDiscOverlapPoint(currPortal->GetPosition(), currPortal->GetInnerRadius(), currActor->GetPosition()))
				{
					currPortal->MoveActorToDestinationMap(currActor);
					
					// Check if the player teleported, if so switch maps
					if (currActor == g_thePlayer)
					{
						m_theAdventure->TransitionToMap(currPortal->GetDestinationMap());
					}

					actorIndex--; // Todo: check this
				}
			}
			// else check to see if we can make it ready
			else
			{
				if (DoDiscsOverlap(currPortal->GetPosition(), currPortal->GetInnerRadius(), currActor->GetPosition(), currActor->GetInnerRadius()))
				{
					didEntityOverlapPortal = true;
				}
			}
		}

		// If no actors overlapped and the portal wasn't ready, it now can be ready
		if (!portalReady && !didEntityOverlapPortal)
		{
			currPortal->SetIsReady(true);
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Checks if the player is standing on an item, for pickup and drawing text
//
void Map::CheckForItemPickup()
{
	bool foundItem = false;
	for (int itemIndex = 0; itemIndex < static_cast<int>(m_items.size()); itemIndex++)
	{
		Item* currItem = m_items[itemIndex];
		if (DoDiscsOverlap(g_thePlayer->GetPosition(), g_thePlayer->GetInnerRadius(), currItem->GetPosition(), currItem->GetInnerRadius()))
		{
			m_itemPlayerIsOn = currItem;
			foundItem = true;
			break;
		}
	}

	if (!foundItem)
	{
		m_itemPlayerIsOn = nullptr;
	}
}


//-----------------------------------------------------------------------------------------------
// Deletes all entities marked for death this frame from both entity lists
// DOES NOT DELETE THE PLAYER
//
void Map::DeleteMarkedEntities()
{
	// Delete Actors
	for (int actorIndex = 0; actorIndex < static_cast<int>(m_actors.size()); actorIndex++)
	{
		Actor* currActor = m_actors[actorIndex];
		if (currActor->IsMarkedForDeletion())
		{
			if (currActor != g_thePlayer)
			{
				m_theAdventure->CheckForVictory(currActor->GetName());	// Check for victory
				RemoveActorFromMap(currActor);
				delete currActor;
				actorIndex--;	// All actors after this one in the list are shifted after deletion, so don't increment
			}
			else
			{
				// Player died, so enter game over state
				if (g_theGame->GetCurrentState() != GAME_STATE_GAMEOVER && g_theGame->GetTransitionState() != GAME_STATE_GAMEOVER)
				{
					g_theGame->StartTransitionToState(GAME_STATE_GAMEOVER, true);
				}			
			}
		}
	}


	// Delete Projectiles
	for (int projectileIndex = 0; projectileIndex < static_cast<int>(m_projectiles.size()); projectileIndex++)
	{
		Projectile* currProjectile = m_projectiles[projectileIndex];
		if (currProjectile->IsMarkedForDeletion())
		{
			RemoveProjectileFromMap(currProjectile);
			delete currProjectile;
			projectileIndex--;	// All projectiles after this one in the list are shifted after deletion, so don't increment
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Constructs the Vertex3D_PCU structs for the given tile and adds them to the list of verts to draw
//
void ConstructAndAddTileVertsToMesh(std::vector<Vertex3D_PCU>& meshVertices, const Tile& tile)
{
	// Get base tile information
	AABB2 tileBounds = tile.GetWorldBounds();
	AABB2 baseTileUVs = tile.GetBaseSpriteUVs();
	Rgba baseTileTint = tile.GetBaseTint();

	// Construct the 4 vertexes for the tile, first for base UVs and tint
	// Top Left
	Vertex3D_PCU topLeft;
	topLeft.m_position = Vector3(tileBounds.mins.x, tileBounds.maxs.y, 0.f);
	topLeft.m_color = baseTileTint;
	topLeft.m_texUVs = baseTileUVs.mins;

	// Top Right
	Vertex3D_PCU topRight;
	topRight.m_position = Vector3(tileBounds.maxs.x, tileBounds.maxs.y, 0.f);
	topRight.m_color = baseTileTint;
	topRight.m_texUVs = Vector2(baseTileUVs.maxs.x, baseTileUVs.mins.y);

	// Bottom Left
	Vertex3D_PCU bottomLeft;
	bottomLeft.m_position = Vector3(tileBounds.mins.x, tileBounds.mins.y, 0.f);
	bottomLeft.m_color = baseTileTint;
	bottomLeft.m_texUVs = Vector2(baseTileUVs.mins.x, baseTileUVs.maxs.y);

	// Bottom Right
	Vertex3D_PCU bottomRight;
	bottomRight.m_position = Vector3(tileBounds.maxs.x, tileBounds.mins.y, 0.f);
	bottomRight.m_color = baseTileTint;
	bottomRight.m_texUVs = baseTileUVs.maxs;


	// Push back the 6 vertices needed to draw the square tile (two triangles)
	meshVertices.push_back(topLeft);
	meshVertices.push_back(bottomLeft);
	meshVertices.push_back(topRight);

	meshVertices.push_back(bottomLeft);
	meshVertices.push_back(bottomRight);
	meshVertices.push_back(topRight);


	// Now change color and UVs for the overlay tile information
	AABB2	overlayTileUVs = tile.GetOverlaySpriteUVs();
	Rgba	overlayTileTint = tile.GetOverlayTint();

	// Top Left
	topLeft.m_color = overlayTileTint;
	topLeft.m_texUVs = overlayTileUVs.mins;

	// Top Right
	topRight.m_color = overlayTileTint;
	topRight.m_texUVs = Vector2(overlayTileUVs.maxs.x, overlayTileUVs.mins.y);

	// Bottom Left
	bottomLeft.m_color = overlayTileTint;
	bottomLeft.m_texUVs = Vector2(overlayTileUVs.mins.x, overlayTileUVs.maxs.y);

	// Bottom Right
	bottomRight.m_color = overlayTileTint;
	bottomRight.m_texUVs = overlayTileUVs.maxs;

	// Push back the second set of vertices
	meshVertices.push_back(topLeft);
	meshVertices.push_back(bottomLeft);
	meshVertices.push_back(topRight);

	meshVertices.push_back(bottomLeft);
	meshVertices.push_back(bottomRight);
	meshVertices.push_back(topRight);
}


//-----------------------------------------------------------------------------------------------
// Renders the tiles that are currently in view of the camera as a single mesh
//
void Map::RenderTiles() const
{
	static std::vector<Vertex3D_PCU> meshVertices;
	meshVertices.clear();	// Clear last frame's vertices	

	for (int tileIndex = 0; tileIndex < static_cast<int>(m_tiles.size()); tileIndex++)
	{
		Tile currTile = m_tiles[tileIndex];

		// Only construct the vertices if the tile is in view
		if (g_theCamera->IsInCameraView(currTile.GetWorldBounds())) 
		{ 
			ConstructAndAddTileVertsToMesh(meshVertices, currTile);
		}
	}

	// Set up texturing and draw the mesh
	g_theRenderer->Enable2DTexture();

	const Texture& tileTexture = TileDefinition::GetTileSheetTexture();

	g_theRenderer->Bind2DTexture(tileTexture);
	g_theRenderer->DrawMeshImmediate(&meshVertices[0], static_cast<int>(meshVertices.size()), PRIMITIVE_TRIANGLES);
	g_theRenderer->Disable2DTexture();
}


//-----------------------------------------------------------------------------------------------
// Draws the pickup text for picking up an item when the player is standing on it
//
void Map::DrawPickupText() const
{
	g_theRenderer->SetOrtho(*g_screenBounds);
	g_theRenderer->DrawTextInBox2D(Stringf("Press X to add %s to inventory.\nPress Y to equip.", m_itemPlayerIsOn->GetName().c_str()), PICKUP_DRAW_BOX, Vector2::ZERO, 0.5f, TEXT_DRAW_SHRINK_TO_FIT);
}


//-----------------------------------------------------------------------------------------------
// For the given entity, checks all adjacent tiles to its position to see if there is collision
//
void Map::CheckEntityForAdjacentTileCollisions(Entity* entity, bool shouldDestroy)
{
	Vector2 entityPosition = entity->GetPosition();
	Tile* currTile = GetTileFromPosition(entityPosition);

	if (currTile == nullptr)
	{
		return;
	}

	IntVector2 currTileCoords = currTile->GetTileCoords();
	Vector2 currTilePositions = Vector2(currTileCoords);	// Getting the coords as floats to prevent having to cast below from ints to floats

	//										 --Adjacent tile coordinates--					--Collision point position, on edge of tile--					
	CheckEntityTileCollision(entity, currTileCoords + IntVector2::STEP_NORTH,		Vector2(entityPosition.x,				currTilePositions.y + 1.f),		shouldDestroy);		// NORTH
	CheckEntityTileCollision(entity, currTileCoords + IntVector2::STEP_EAST,		Vector2(currTilePositions.x + 1.f,		entityPosition.y),				shouldDestroy);		// EAST
	CheckEntityTileCollision(entity, currTileCoords + IntVector2::STEP_SOUTH,		Vector2(entityPosition.x,				currTilePositions.y),			shouldDestroy);		// SOUTH
	CheckEntityTileCollision(entity, currTileCoords + IntVector2::STEP_WEST,		Vector2(currTilePositions.x,			entityPosition.y),				shouldDestroy);		// WEST
	CheckEntityTileCollision(entity, currTileCoords + IntVector2::STEP_NORTHEAST,	Vector2(currTilePositions.x + 1.f,		currTilePositions.y + 1.f),		shouldDestroy);	    // NORTHEAST
	CheckEntityTileCollision(entity, currTileCoords + IntVector2::STEP_SOUTHEAST,	Vector2(currTilePositions.x + 1.f,		currTilePositions.y),			shouldDestroy);		// SOUTHEAST
	CheckEntityTileCollision(entity, currTileCoords + IntVector2::STEP_SOUTHWEST,	Vector2(currTilePositions.x,			currTilePositions.y),			shouldDestroy);		// SOUTHWEST
	CheckEntityTileCollision(entity, currTileCoords + IntVector2::STEP_NORTHWEST,	Vector2(currTilePositions.x,			currTilePositions.y + 1.f),		shouldDestroy);		// NORTHWEST
}


//-----------------------------------------------------------------------------------------------
// Checks the entity against the adjacent tile and corrects its position if there is collision
//
void Map::CheckEntityTileCollision(Entity* entity, const IntVector2& adjTileCoords, const Vector2& collisionPoint, bool shouldDestroy)
{
	Tile* adjTile = GetTileFromCoords(adjTileCoords);
	if (!EntityCanEnterTile(entity, adjTile))
	{
		// Check if we overlap the collision point, and if so correct it
		if (DoesDiscOverlapPoint(entity->GetPosition(), entity->GetInnerRadius(), collisionPoint))
		{
			// If we should destroy it, mark it for destruction
			if (shouldDestroy)
			{
				entity->SetMarkedForDeath(true);
			}
			else
			{
				// Else fix the entity's position so it doesn't contain the collision point
				Vector2 adjustDirection = (entity->GetPosition() - collisionPoint);

				// For testing, ensure we have entities not directly on a collision point (most likely occurs from a bug)
				ASSERT_OR_DIE(adjustDirection != Vector2::ZERO, Stringf("Error: Entity \"%s\" was on top of a collision point.", entity->GetName().c_str()));

				float distance = adjustDirection.NormalizeAndGetLength();
				float adjustMagnitude = (entity->GetInnerRadius() - distance);
				entity->SetPosition(entity->GetPosition() + (adjustDirection * adjustMagnitude));
			}
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Returns true if the entity is a valid, alive entity on this map
//
bool Map::IsEntityAlive(Entity* entity) const
{
	return ((entity != nullptr) && (!entity->IsMarkedForDeletion()));
}


//-----------------------------------------------------------------------------------------------
// Returns true if this entity can enter this tile (walk, swim, or fly)
//
bool Map::EntityCanEnterTile(Entity* entity, const Tile* tile)
{
	if (tile == nullptr) { return false; }

	const TileDefinition* currDefinition = tile->GetDefinition();
	bool walkable	= (entity->CanWalk() && currDefinition->AllowsWalking());
	bool flyable	= (entity->CanFly() && currDefinition->AllowsFlying());
	bool swimmable	= (entity->CanSwim() && currDefinition->AllowsSwimming());

	return (walkable || flyable || swimmable);
}
