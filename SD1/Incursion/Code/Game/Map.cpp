/************************************************************************/
/* Project: Incursion
/* File: Map.cpp
/* Author: Andrew Chase
/* Date: September 25th, 2017
/* Bugs: None
/* Description: Implementation of the Map class
/************************************************************************/
#include "Game/Map.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Camera.hpp"
#include "Game/NpcTank.hpp"
#include "Game/NpcTurret.hpp"
#include "Game/PlayerTank.hpp"
#include "Game/Bullet.hpp"
#include "Game/Explosion.hpp"
#include "Game/PickUp.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/Disc2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Input/InputSystem.hpp"

const Vector2 Map::TILE_CENTER_OFFSET = Vector2(0.5f, 0.5f);
const IntVector2 Map::PLAYER_START_AREA_SIZE = IntVector2(2, 2);

// Map intial states, for explicit construction
const IntVector2 Map::MAP_ZERO_DIMENSIONS		= IntVector2(25, 35);
const IntVector2 Map::MAP_ZERO_PLAYER_START		= IntVector2(4, 4);
const IntVector2 Map::MAP_ZERO_LEVEL_END		= IntVector2(21, 31);
const int		 Map::MAP_ZERO_NUM_TANKS		= GetRandomIntInRange(12, 20);
const int		 Map::MAP_ZERO_NUM_TURRETS		= GetRandomIntInRange(10, 15);
				 
const IntVector2 Map::MAP_ONE_DIMENSIONS		= IntVector2(45, 30);
const IntVector2 Map::MAP_ONE_PLAYER_START		= IntVector2(40, 25);
const IntVector2 Map::MAP_ONE_LEVEL_END			= IntVector2(4, 6);
const int		 Map::MAP_ONE_NUM_TANKS			= GetRandomIntInRange(25, 30);
const int		 Map::MAP_ONE_NUM_TURRETS		= GetRandomIntInRange(15, 22);
				 
const IntVector2 Map::MAP_TWO_DIMENSIONS		= IntVector2(50, 50);
const IntVector2 Map::MAP_TWO_PLAYER_START		= IntVector2(30, 45);
const IntVector2 Map::MAP_TWO_LEVEL_END			= IntVector2(46, 3);
const int		 Map::MAP_TWO_NUM_TANKS			= GetRandomIntInRange(30, 40);
const int		 Map::MAP_TWO_NUM_TURRETS		= GetRandomIntInRange(25, 35);

//-----------------------------------------------------------------------------------------------
// Constructor - also currently initializes the map to a random tile layout
//
Map::Map(int mapNumber)
{
	switch (mapNumber)
	{
	case 0:
		m_dimensions				= MAP_ZERO_DIMENSIONS;
		m_playerStartCoords			= MAP_ZERO_PLAYER_START;
		m_levelEndCoords			= MAP_ZERO_LEVEL_END;
		m_numInitialEnemyTanks		= MAP_ZERO_NUM_TANKS;
		m_numIntialEnemyTurrets		= MAP_ZERO_NUM_TURRETS;
		break;
	case 1:
		m_dimensions				= MAP_ONE_DIMENSIONS;
		m_playerStartCoords			= MAP_ONE_PLAYER_START;
		m_levelEndCoords			= MAP_ONE_LEVEL_END;
		m_numInitialEnemyTanks		= MAP_ONE_NUM_TANKS;
		m_numIntialEnemyTurrets		= MAP_ONE_NUM_TURRETS;
		break;
	case 2:
		m_dimensions				= MAP_TWO_DIMENSIONS;
		m_playerStartCoords			= MAP_TWO_PLAYER_START;
		m_levelEndCoords			= MAP_TWO_LEVEL_END;
		m_numInitialEnemyTanks		= MAP_TWO_NUM_TANKS;
		m_numIntialEnemyTurrets		= MAP_TWO_NUM_TURRETS;
		break;
	default:
		break;
	}

	m_mapNumber = mapNumber;
}


//-----------------------------------------------------------------------------------------------
// Deletes all entities
//
Map::~Map()
{
	m_isBeingDestroyed = true;

	for (int entityIndex = 0; entityIndex < static_cast<int>(m_entities.size()); entityIndex++)
	{
		// Player was already deleted by Game, so don't delete that pointer a second time
		if (m_entities[entityIndex] != g_thePlayer)
		{
			delete m_entities[entityIndex];
		}
	}

	// No need to worry about the pointers in both the entity vector and the entities by type vector,
	// as both will be cleaned up now
}

//-----------------------------------------------------------------------------------------------
// Calls Update() on all entities and perform physics checks
//
void Map::Update(float deltaTime)
{
	ProcessMapInput();						// Update map based on map input
	UpdateEntities(deltaTime);				// Updates all Entities' state
	CheckAllEntityEntityCollisions();		// Checks for collisions between entities, and fixes them
	CheckAllEntityTileCollisions();			// Checks for collisions between entities and tiles, and fixes them
	CheckPickUpPlayerCollisions();			// Checks if the player collided with any pickups
	CheckEntityEntityInteractions();		// Checks for bullet collisions against tanks
	DeleteMarkedEntities();					// Deletes all entities marked for delete this frame
}


//-----------------------------------------------------------------------------------------------
// Calls Render on all tiles and entities
//
void Map::Render() const
{
	// Set the view to be on the player, with the correct aspect
	if (g_theInputSystem->IsKeyPressed(InputSystem::KEYBOARD_F3))
	{
		g_theCamera->SetMapView(m_dimensions);
	}
	else
	{
		g_theCamera->SetDefaultView();
	}

	for (int i = 0; i < static_cast<int>(m_tiles.size()); i++)
	{
		m_tiles[i].Render();
	}

	if (m_drawGridLines)
	{
		DrawGridLines();
	}

	for (int entityType = 0; entityType < NUM_ENTITY_TYPES; ++entityType)
	{
		for (int entityIndex = 0; entityIndex < static_cast<int>(m_entitiesByType[entityType].size()); ++entityIndex)
		{
			Entity* currEntity = m_entitiesByType[entityType][entityIndex];
			if (IsEntityAlive(currEntity))
			{
				currEntity->Render();
			}
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Returns a TileType based on the given tileCoords. Tiles on the edge of the map are stone by
// default, whereas interior tiles are randomly chosen as grass or stone
//
TileType Map::GenerateTileType(const IntVector2& tileCoords)
{
	// Ensure the start tile is a grass square
	if (IsTileInPlayerStartArea(tileCoords))
	{
		return TILE_TYPE_PLAYER_START;
	}
	else if (IsTileTheExitTile(tileCoords))
	{
		return TILE_TYPE_END_LEVEL;
	}

	TileType tileType = TILE_TYPE_GRASS;

	// Tile on the edge of the map
	if (IsTileOnEdgeOfMap(tileCoords))
	{
		tileType = TILE_TYPE_STONE;
	}
	// Else 25% chance of being stone
	else if (CheckRandomChance(0.15f))
	{
		tileType = TILE_TYPE_STONE;
	}
	else if (CheckRandomChance(0.05f))
	{
		tileType = TILE_TYPE_WATER;
	}

	return tileType;
}


//-----------------------------------------------------------------------------------------------
// Returns true if the tile coords specify a tile near where the player starts
//
bool Map::IsTileInPlayerStartArea(const IntVector2& tileCoords) const
{
	IntVector2 minCoords = m_playerStartCoords - IntVector2(1, 1);
	IntVector2 maxCoords = m_playerStartCoords + PLAYER_START_AREA_SIZE;

	if (tileCoords.x >= minCoords.x && tileCoords.x < maxCoords.x && tileCoords.y >= minCoords.y && tileCoords.y < maxCoords.y)
	{
		return true;
	}

	return false;
}


//-----------------------------------------------------------------------------------------------
// Returns true if the tile coords specify a tile on the edge wall of the map
//
bool Map::IsTileOnEdgeOfMap(const IntVector2& tileCoords) const
{
	IntVector2 minCoords = IntVector2(OUTSIDE_WALL_THICKNESS, OUTSIDE_WALL_THICKNESS);
	IntVector2 maxCoords = (m_dimensions - minCoords);
	if (tileCoords.x < minCoords.x || tileCoords.x >= maxCoords.x || tileCoords.y < minCoords.y || tileCoords.y >= maxCoords.y)
	{
		return true;
	}

	return false;
}


//-----------------------------------------------------------------------------------------------
// Returns true if tileCoords specify the level exit tile
//
bool Map::IsTileTheExitTile(const IntVector2& tileCoords) const
{
	return (tileCoords == m_levelEndCoords);
}


//-----------------------------------------------------------------------------------------------
// Adds the entity to the list of entities on this map
// If the entity added is the player, ensure they spawn on the start space
//
void Map::AddEntityToMap(Entity* entityToAdd)
{
	entityToAdd->SetMap(this);
	m_entities.push_back(entityToAdd);
	m_entitiesByType[entityToAdd->GetEntityType()].push_back(entityToAdd);

	if (entityToAdd == g_thePlayer)
	{
		g_thePlayer->SetPosition(m_playerStartCoords.GetAsFloats() + TILE_CENTER_OFFSET);
	}
}


//-----------------------------------------------------------------------------------------------
// Finds the player in this map, and removes the reference to it from this map and returns it
// Returns null if the player cannot be found on this map
//
void Map::RemovePlayerFromMap()
{
	std::vector<Entity*>& playerVector = m_entitiesByType[ENTITY_TYPE_PLAYERTANK];

	for (int entityIndex = 0; entityIndex < static_cast<int>(playerVector.size()); entityIndex++)
	{
		// Comparing pointers
		if (playerVector[entityIndex] == g_thePlayer)
		{
			playerVector.erase(playerVector.begin() + entityIndex);
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Returns the nearest hostile target in range and line of sight to hostilePosition
//
Entity* Map::FindBestTarget(Entity* attackingEntity)
{
	Entity* bestTarget = nullptr;
	float bestDistance = -1.f;

	for (int entityIndex = 0; entityIndex < static_cast<int>(m_entities.size()); entityIndex++)
	{
		Entity* currEntity = m_entities[entityIndex];

		// Ensure the target is alive
		if (currEntity->IsMarkedForDeath())
		{
			continue;
		}

		// If the entity is just a bullet, pickup, or explosion, continue
		EntityType currEntityType = currEntity->GetEntityType();
		if (currEntityType == ENTITY_TYPE_BULLET || currEntityType == ENTITY_TYPE_EXPLOSION || currEntityType == ENTITY_TYPE_PICKUP)
		{
			continue;
		}
		// Else if the entity is not hostile to the attacker, continue
		else if (currEntity->GetFaction() == attackingEntity->GetFaction())
		{
			continue;
		}
		
		// Calculate length twice here, once in HasLineOfSight and once here
		float distanceBetween = (attackingEntity->GetPosition() - currEntity->GetPosition()).GetLength();

		if (HasLineOfSight(attackingEntity->GetPosition(), currEntity->GetPosition(), 20))
		{
			if (bestTarget == nullptr || (distanceBetween < bestDistance))
			{
				bestTarget = currEntity;
				bestDistance = distanceBetween;
			}
		}
	}

	return bestTarget;
}

//-----------------------------------------------------------------------------------------------
// Returns the Tile that contains the given position on the map
//
Tile Map::GetTileFromPosition(const Vector2& position) const
{
	IntVector2 tileCoords = IntVector2(static_cast<int>(position.x), static_cast<int>(position.y));

	return GetTileFromCoords(tileCoords);
}


//-----------------------------------------------------------------------------------------------
// Return the Tile given the tileCoords
//
Tile Map::GetTileFromCoords(const IntVector2& tileCoords) const
{
	int index = (m_dimensions.x * tileCoords.y) + tileCoords.x;
	
	// If an entity clips out of bounds and we try to find a non-existant tile, just return
	// the first solid tile of the map
	if (index >= static_cast<int>(m_tiles.size()) || index < 0)
	{
		index = 0;
	}
	return m_tiles[index];
}


//-----------------------------------------------------------------------------------------------
// Updates the map state given the map-based input
//
void Map::ProcessMapInput()
{
	if (g_theInputSystem->WasKeyJustPressed(InputSystem::KEYBOARD_F2))
	{
		m_drawGridLines = !m_drawGridLines;
	}

	if (g_theInputSystem->WasKeyJustPressed('H'))
	{
		SpawnEntity(Vector2(5.5f, 5.5f), 0.f, FACTION_PLAYER, ENTITY_TYPE_NPCTANK);
	}
}


//-----------------------------------------------------------------------------------------------
// Calls Update on all entities in the map
//
void Map::UpdateEntities(float deltaTime)
{
	for (int entityIndex = 0; entityIndex < static_cast<int>(m_entities.size()); ++entityIndex)
	{
		Entity* currEntity = m_entities[entityIndex];

		if (currEntity != nullptr)
		{
			if (currEntity->GetEntityType() == ENTITY_TYPE_PLAYERTANK)
			{
				currEntity->Update(deltaTime);

			}
			else
			{
				currEntity->Update(deltaTime);

			}
		}	
	}
}


//-----------------------------------------------------------------------------------------------
// Checks if any two non-bullet entities are within eachother, and if so pushes them apart evenly
//
void Map::CheckAllEntityEntityCollisions()
{
	for (int firstIndex = 0; firstIndex < static_cast<int>(m_entities.size()) - 1; firstIndex++)
	{
		Entity* firstEntity = m_entities[firstIndex];
		EntityType firstType = firstEntity->GetEntityType();
		if (firstType == ENTITY_TYPE_PICKUP || firstType == ENTITY_TYPE_BULLET || !firstEntity->IsUsingPhysics())
		{
			continue;
		}

		for (int secondIndex = firstIndex + 1; secondIndex < static_cast<int>(m_entities.size()); secondIndex++)
		{
			Entity* secondEntity = m_entities[secondIndex];
			EntityType secondType = secondEntity->GetEntityType();
			if (secondType == ENTITY_TYPE_PICKUP || secondType == ENTITY_TYPE_BULLET || !secondEntity->IsUsingPhysics())
			{
				continue;
			}

			// Check to see if these two entities are overlapping
			if (DoDiscsOverlap(firstEntity->GetPosition(), firstEntity->GetInnerRadius(), secondEntity->GetPosition(), secondEntity->GetInnerRadius()))
			{
				CorrectEntityEntityCollision(firstEntity, secondEntity);
			}
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Checks for and corrects collisions between entities and the tiles
//
void Map::CheckAllEntityTileCollisions()
{
	for (Entity* curr : m_entities)
	{
		EntityType currType = curr->GetEntityType();
		if (IsEntityAlive(curr) && currType != ENTITY_TYPE_BULLET && currType != ENTITY_TYPE_PICKUP && curr->IsUsingPhysics())
		{
			CheckEntityForTileCollisions(curr);
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Pushes the two given entities out of eachother evenly
//
void Map::CorrectEntityEntityCollision(Entity* firstEntity, Entity* secondEntity)
{
	float sumOfRadii = (firstEntity->GetInnerRadius() + secondEntity->GetInnerRadius());
	Vector2 directionToSecond = (secondEntity->GetPosition() - firstEntity->GetPosition());
	float distance = directionToSecond.NormalizeAndGetLength();

	// Check for if the two entities are on top of each other exactly, and if so just push them out in a random direction
	if (distance == 0.f)
	{
		directionToSecond = Vector2(GetRandomFloatZeroToOne(), GetRandomFloatZeroToOne()).GetNormalized();
	}

	// Total distance for the two entities to be separated by
	float totalCorrectiveMagnitude = (sumOfRadii - distance);

	Vector2 firstCorrection = Vector2::ZERO;
	Vector2 secondCorrection = Vector2::ZERO;

	// Check to see if the entities are turrets, and if so don't move them
	if (firstEntity->GetEntityType() == ENTITY_TYPE_TURRET)
	{
		secondCorrection = (directionToSecond * totalCorrectiveMagnitude);
	}
	else if (secondEntity->GetEntityType() == ENTITY_TYPE_TURRET)
	{
		firstCorrection = (-1.0 * directionToSecond * totalCorrectiveMagnitude);
	}
	// If both aren't turrets than move both equally apart
	else
	{
		firstCorrection = (-0.5f * directionToSecond * totalCorrectiveMagnitude);
		secondCorrection = (0.5f * directionToSecond * totalCorrectiveMagnitude);
	}

	// Adjust both entities' position to be correct
	firstEntity->SetPosition(firstEntity->GetPosition() + firstCorrection);
	secondEntity->SetPosition(secondEntity->GetPosition() + secondCorrection);
}


//-----------------------------------------------------------------------------------------------
// Performs a raycast from startPosition in direction out maxDistance, and returns a result as soon
// as a collision is found
//
RaycastResult Map::Raycast(const Vector2& startPosition, const Vector2& direction, float maxDistance) const
{
	Vector2 normalizedDirection = direction.GetNormalized();

	int numSteps = static_cast<int>(maxDistance * RAYCAST_STEPS_PER_TILE);
	Vector2 singleStep = (normalizedDirection / static_cast<float>(RAYCAST_STEPS_PER_TILE));

	IntVector2 previousTileCoords = GetTileFromPosition(startPosition).GetCoordinates();

	// Iterate the required number of steps to cover maxDistance
	for (int stepNumber = 0; stepNumber <= numSteps; ++stepNumber)
	{
		// Recalculate the step each time to prevent floating point error build up
		Vector2 currPosition = startPosition + (singleStep * static_cast<float>(stepNumber));
		IntVector2 currTileCoords = GetTileFromPosition(currPosition).GetCoordinates();

		// If we haven't changed tiles, then we know there is no new information, so continue
		if (currTileCoords == previousTileCoords)
		{
			continue;
		}

		if (!GetTileFromCoords(currTileCoords).CanSeeThrough())
		{
			// Make a RaycastResult for this impact
			RaycastResult impactResult;
			impactResult.m_didImpact = true;
			impactResult.m_impactPosition = currPosition;
			impactResult.m_impactTileCoords = currTileCoords;
			impactResult.m_impactDistance = (static_cast<float>(stepNumber) * singleStep).GetLength();
			impactResult.m_impactFraction = (impactResult.m_impactDistance / maxDistance);
			impactResult.m_impactNormal = (currTileCoords - previousTileCoords).GetAsFloats();
			return impactResult;
		}

		// Otherwise update our previous tile coordinates
		previousTileCoords = currTileCoords;
	}

	// There was no impact, so return a RaycastResult for no impact
	RaycastResult noImpactResult;
	noImpactResult.m_didImpact = false;																			// No impact
	noImpactResult.m_impactPosition = startPosition + (normalizedDirection * maxDistance);						// The end point of our raycast
	noImpactResult.m_impactTileCoords = GetTileFromPosition(noImpactResult.m_impactPosition).GetCoordinates();  // Tile that contains the end point
	noImpactResult.m_impactDistance = maxDistance;																// Ray went the full distance
	noImpactResult.m_impactFraction = 1;																		// Ray went the full distance
	noImpactResult.m_impactNormal = Vector2::ZERO;																// There was no impact, so no impact normal
	
	return noImpactResult;
}


//-----------------------------------------------------------------------------------------------
// Returns true if there is a clear line of sight between startPos and endPos, and if they are within
// viewDistance from each other
//
bool Map::HasLineOfSight(const Vector2& startPos, const Vector2& endPos, float viewDistance) const
{
	Vector2 directionToCast = (endPos - startPos);
	float distance = directionToCast.NormalizeAndGetLength();

	RaycastResult raycastResult = Raycast(startPos, directionToCast, distance);

	return (!raycastResult.m_didImpact && (distance < viewDistance));
}


//-----------------------------------------------------------------------------------------------
// Checks to see if any bullets collide with non-friendly non-bullet entities
//
void Map::CheckEntityEntityInteractions()
{
	// Iterate across all bullets
	for (int bulletIndex = 0; bulletIndex < static_cast<int>(m_entitiesByType[ENTITY_TYPE_BULLET].size()); bulletIndex++)
	{
		Entity* currBullet = m_entitiesByType[ENTITY_TYPE_BULLET][bulletIndex];

		// For this bullet, check against all other non-bullet entities for collision

		// Against all players
		if (!currBullet->IsMarkedForDeath())
		{
			CheckBulletAgainstPlayers(currBullet);
		}

		if (!currBullet->IsMarkedForDeath())
		{
			CheckBulletAgainstNpcTanks(currBullet);
		}
		
		if (!currBullet->IsMarkedForDeath())
		{
			CheckBulletAgainstNpcTurrets(currBullet);
		}	
	}
}


//-----------------------------------------------------------------------------------------------
// Checks to see if the given bullet is of a different faction than the player, and if so checks for
// collision and updates each accordingly
//
void Map::CheckBulletAgainstPlayers(Entity* currBullet)
{
	int numPlayers = static_cast<int>(m_entitiesByType[ENTITY_TYPE_PLAYERTANK].size());

	for (int playerIndex = 0; playerIndex < numPlayers; playerIndex++)
	{
		PlayerTank* currPlayer = static_cast<PlayerTank*>(m_entitiesByType[ENTITY_TYPE_PLAYERTANK][playerIndex]);

		// If they are different faction, and if they are not marked for death
		if (!currPlayer->IsInGodMode() && (currBullet->GetFaction() != currPlayer->GetFaction()) && !currPlayer->IsMarkedForDeath())
		{
			if (DoDiscsOverlap(currBullet->GetPosition(), currBullet->GetInnerRadius(), currPlayer->GetPosition(), currPlayer->GetInnerRadius()))
			{
				currPlayer->TakeDamage(1);
				currBullet->TakeDamage(9999);
			}
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Checks to see if the given bullet collides with an opposing faction npc tank, and if so 
// deals damage accordingly
//
void Map::CheckBulletAgainstNpcTanks(Entity* currBullet)
{
	int numTanks = static_cast<int>(m_entitiesByType[ENTITY_TYPE_NPCTANK].size());

	for (int npcTankIndex = 0; npcTankIndex < numTanks; npcTankIndex++)
	{
		Entity* currNpcTank = m_entitiesByType[ENTITY_TYPE_NPCTANK][npcTankIndex];

		// If they are different faction, and if they aren't marked for death
		if (currBullet->GetFaction() != currNpcTank->GetFaction() && !currNpcTank->IsMarkedForDeath())
		{
			if (DoDiscsOverlap(currBullet->GetPosition(), currBullet->GetInnerRadius(), currNpcTank->GetPosition(), currNpcTank->GetInnerRadius()))
			{
				currNpcTank->TakeDamage(1);
				currBullet->TakeDamage(9999);
			}
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Checks to see if the given bullet collides with an opposing faction npc turret, and if so 
// deals damage accordingly
//
void Map::CheckBulletAgainstNpcTurrets(Entity* currBullet)
{
	int numTurrets = static_cast<int>(m_entitiesByType[ENTITY_TYPE_TURRET].size());

	for (int npcTurretIndex = 0; npcTurretIndex < numTurrets; npcTurretIndex++)
	{
		Entity* currNpcTurret = m_entitiesByType[ENTITY_TYPE_TURRET][npcTurretIndex];

		// If they are different faction, and if they aren't marked for death
		if (currBullet->GetFaction() != currNpcTurret->GetFaction() && !currNpcTurret->IsMarkedForDeath())
		{
			if (DoDiscsOverlap(currBullet->GetPosition(), currBullet->GetInnerRadius(), currNpcTurret->GetPosition(), currNpcTurret->GetInnerRadius()))
			{
				currNpcTurret->TakeDamage(1);
				currBullet->TakeDamage(9999);
			}
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Checks to see if the player touched any pick ups this frame
//
void Map::CheckPickUpPlayerCollisions()
{
	std::vector<Entity*>& playerVector = m_entitiesByType[ENTITY_TYPE_PLAYERTANK];
	for (int playerIndex = 0; playerIndex < static_cast<int>(playerVector.size()); playerIndex++)
	{
		PlayerTank* currPlayer = static_cast<PlayerTank*>(playerVector[playerIndex]);
		if (!IsEntityAlive(currPlayer))
		{
			continue;
		}

		std::vector<Entity*>& pickUpVector = m_entitiesByType[ENTITY_TYPE_PICKUP];
		for (int pickUpIndex = 0; pickUpIndex < static_cast<int>(pickUpVector.size()); pickUpIndex++)
		{
			PickUp* currPickUp = static_cast<PickUp*>(pickUpVector[pickUpIndex]);

			if (!IsEntityAlive(currPickUp))
			{
				continue;
			}

			// Both are not null and alive, so check for overlap
			if (DoDiscsOverlap(currPlayer->GetPosition(), currPlayer->GetInnerRadius(), currPickUp->GetPosition(), currPickUp->GetInnerRadius()))
			{
				// Overlapped, so use the pickup (automatically marks the pickup for deletion)
				currPickUp->ApplyPickUpEffectToPlayer();
			}
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Deletes all entities marked for death this frame from both entity lists
//
void Map::DeleteMarkedEntities()
{
	for (int entityIndex = 0; entityIndex < static_cast<int>(m_entities.size()); entityIndex++)
	{
		Entity* currEntity = m_entities[entityIndex];
		if (currEntity->GetEntityType() != ENTITY_TYPE_PLAYERTANK && currEntity->IsMarkedForDeath())
		{
			// Find the corresponding pointer in the entity-by-type array
			std::vector<Entity*>& typeVector = m_entitiesByType[currEntity->GetEntityType()];

			for (int entityTypeIndex = 0; entityTypeIndex < static_cast<int>(typeVector.size()); entityTypeIndex++)
			{
				if (typeVector[entityTypeIndex] == currEntity)
				{
					typeVector.erase(typeVector.begin() + entityTypeIndex);
					m_entities.erase(m_entities.begin() + entityIndex);

					delete currEntity;
					break;
				}
			}
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Checks if, for the current Tile and collision case, whether the adjacent tile is solid
// (whether an entity can move through it)
//
bool Map::IsAdjacentTileSolid(const Tile& currTile, TileCollisionCase currCollisionCase) const
{
	IntVector2 adjTileCoords = currTile.GetCoordinates();

	// Based on the case, we look at a different adjacent tile, so increment coordinates accordingly
	switch(currCollisionCase)
	{
	case TILE_COLLISION_CASE_NORTH:
	{
		adjTileCoords.y++;
		break;
	}
	case TILE_COLLISION_CASE_SOUTH:
	{
		adjTileCoords.y--;
		break;
	}
	case TILE_COLLISION_CASE_EAST:
	{
		adjTileCoords.x++;
		break;
	}
	case TILE_COLLISION_CASE_WEST:
	{
		adjTileCoords.x--;
		break;
	}
	case TILE_COLLISION_CASE_NORTHEAST:
	{
		adjTileCoords.x++;
		adjTileCoords.y++;
		break;
	}
	case TILE_COLLISION_CASE_NORTHWEST:
	{
		adjTileCoords.x--;
		adjTileCoords.y++;
		break;
	}
	case TILE_COLLISION_CASE_SOUTHEAST:
	{
		adjTileCoords.x++;
		adjTileCoords.y--;
		break;
	}
	case TILE_COLLISION_CASE_SOUTHWEST:
	{
		adjTileCoords.x--;
		adjTileCoords.y--; 
		break;
	}
	default:
		break;
	}

	if (adjTileCoords.x < 0 || adjTileCoords.x >= m_dimensions.x || adjTileCoords.y < 0 || adjTileCoords.y >= m_dimensions.y)
	{
		return false;
	}

	// Get the adjacent tile
	Tile adjTile = GetTileFromCoords(adjTileCoords);

	return (!adjTile.CanDriveOver());
	
}


//-----------------------------------------------------------------------------------------------
// Returns true if the player entered the exit level tile
//
bool Map::DidPlayerFinishLevel() const
{
	Vector2 playerPosition = g_thePlayer->GetPosition();
	Tile currentTile = GetTileFromPosition(playerPosition);

	if (currentTile.GetCoordinates() == m_levelEndCoords)
	{
		return true;
	}
	else
	{
		return false;
	}
}


//-----------------------------------------------------------------------------------------------
// Returns true if the map destructor for this map has been called
//
bool Map::IsBeingDestroyed() const
{
	return m_isBeingDestroyed;
}


//-----------------------------------------------------------------------------------------------
// Draws a black grid over all other renders, to make it more obvious
//
void Map::DrawDebugInfo() const
{
	for (int i = 0; i < static_cast<int>(m_entities.size()); i++)
	{
		m_entities[i]->DrawDebugInfo();
	}
}


//-----------------------------------------------------------------------------------------------
// Sets up the initial state of the map, by creating the NxM grid of tiles and spawning
// initial NPC entities
//
void Map::SetupInitialState()
{
	// Make m_dimensions map of dummy tiles
	for (int yCoordinate = 0; yCoordinate < m_dimensions.y; yCoordinate++)
	{
		for (int xCoordinate = 0; xCoordinate < m_dimensions.x; xCoordinate++)
		{
			IntVector2 tileCoordinate = IntVector2(xCoordinate, yCoordinate);
			TileType tileType = GenerateTileType(tileCoordinate);
			m_tiles.push_back(Tile(tileCoordinate, tileType));
		}
	}

	// Spawn the npcs randomly
	SpawnNpcEntities();
}


//-----------------------------------------------------------------------------------------------
// Spawns an entity of the given type and adds it to this map's entity lists
//
Entity* Map::SpawnEntity(Vector2 spawnPosition, float spawnOrientation, Faction entityFaction, EntityType typeToSpawn)
{
	Entity* newEntity = nullptr;

	switch (typeToSpawn)
	{
	case ENTITY_TYPE_PLAYERTANK:
		newEntity = new PlayerTank();
		break;
	case ENTITY_TYPE_NPCTANK:
		newEntity = new NpcTank(spawnPosition, entityFaction, static_cast<NpcTankType>(GetRandomIntLessThan(NUM_NPC_TANK_TYPES)), this);
		break;
	case ENTITY_TYPE_TURRET:
		newEntity = new NpcTurret(spawnPosition, entityFaction, this);
		break;
	case ENTITY_TYPE_BULLET:
		newEntity = new Bullet(spawnPosition, spawnOrientation, entityFaction, this);
		break;
	case ENTITY_TYPE_PICKUP:
		newEntity = new PickUp(spawnPosition, static_cast<PickUpType>(GetRandomIntLessThan(NUM_PICKUP_TYPES)), this);
		break;
	default:
		break;
	}

	return newEntity;
}


//-----------------------------------------------------------------------------------------------
// Spawns an explosion animation given the spawn information
//
Entity* Map::SpawnExplosion(Vector2 spawnPosition, float radius, float durationSeconds, bool playSoundEffect)
{
	if (playSoundEffect)
	{
		PlayRandomExplosionSound();
	}

	return (new Explosion(spawnPosition, radius, durationSeconds, this));
}


//-----------------------------------------------------------------------------------------------
// Plays one of the 5 explosion sounds randomly, with a random speed and fixed volume
//
void Map::PlayRandomExplosionSound() const
{
	// Get all the explosion sounds
	SoundID explosionSounds[5];
	explosionSounds[0] = g_theAudioSystem->CreateOrGetSound(EXPLOSION1_SOUND_PATH);
	explosionSounds[1] = g_theAudioSystem->CreateOrGetSound(EXPLOSION2_SOUND_PATH);
	explosionSounds[2] = g_theAudioSystem->CreateOrGetSound(EXPLOSION3_SOUND_PATH);
	explosionSounds[3] = g_theAudioSystem->CreateOrGetSound(EXPLOSION4_SOUND_PATH);
	explosionSounds[4] = g_theAudioSystem->CreateOrGetSound(EXPLOSION5_SOUND_PATH);

	// Pick a random one and play it
	int randomIndex = GetRandomIntLessThan(5);
	float randomSpeed = GetRandomFloatInRange(0.5f, 1.5f);
	g_theAudioSystem->PlaySound(explosionSounds[randomIndex], false, 0.5f, 0.f, randomSpeed);
}


//-----------------------------------------------------------------------------------------------
// Returns a random spawn position of an entity, which is the center of a non-solid tile
//
std::vector<IntVector2> Map::GetRandomSpawnPositions()
{
	std::vector<IntVector2> validSpawnTileCoords;

	for (int yCoordinate = 0; yCoordinate < m_dimensions.y; ++yCoordinate)
	{
		for (int xCoordinate = 0; xCoordinate < m_dimensions.x; ++xCoordinate)
		{
			IntVector2 currTileCoords = IntVector2(xCoordinate, yCoordinate);
			if (GetTileFromCoords(currTileCoords).CanSpawnOn())
			{
				validSpawnTileCoords.push_back(currTileCoords);
			}
		}
	}

	return validSpawnTileCoords;
}


//-----------------------------------------------------------------------------------------------
// Checks all possible collision cases between this entity and all nearby tiles
//
void Map::CheckEntityForTileCollisions(Entity* entity)
{
	Tile currTile = GetTileFromPosition(entity->GetPosition());

	// Check all 8 adjacent tiles, and if any are solid check for collision
	// If there is collision, fix it
	for (int i = 0; i < NUM_TILE_COLLISION_CASES; i++)
	{
		TileCollisionCase currCollisionCase = static_cast<TileCollisionCase>(i);
		if (IsAdjacentTileSolid(currTile, currCollisionCase))
		{
			Vector2 collisionPoint = currTile.GetCollisionPoint(entity->GetPosition(), currCollisionCase);
			CheckFixedPointCollision(entity, collisionPoint);
		}
	}

}


//-----------------------------------------------------------------------------------------------
// Checks if an entity contains the collision point, and if so corrects the entity's position to
// not contain it
//
void Map::CheckFixedPointCollision(Entity* entity, Vector2 collisionPoint)
{
	if (DoesDiscOverlapPoint(entity->GetPosition(), entity->GetInnerRadius(), collisionPoint))
	{
		// Fix the entity's position so it doesn't contain the collision point
		Vector2 adjustDirection = (entity->GetPosition() - collisionPoint);
		float distance = adjustDirection.NormalizeAndGetLength();
		float adjustMagnitude = (entity->GetInnerRadius() - distance);
		entity->SetPosition(entity->GetPosition() + (adjustDirection * adjustMagnitude));
	}
}


//-----------------------------------------------------------------------------------------------
// Draws black lines on the screen to outline the grid
//
bool Map::IsEntityAlive(Entity* entity) const
{
	return (entity != nullptr && !entity->IsMarkedForDeath());
}


//-----------------------------------------------------------------------------------------------
// Populates the map with npc entities
//
void Map::SpawnNpcEntities()
{
	std::vector<IntVector2> validTileCoords = GetRandomSpawnPositions();

	
	for (int i = 0; i < m_numInitialEnemyTanks; i++)
	{
		int randomIndex = GetRandomIntLessThan(validTileCoords.size());
		IntVector2 tileCoords = validTileCoords[randomIndex];
		validTileCoords.erase(validTileCoords.begin() + randomIndex);
		Vector2 spawnPosition = tileCoords.GetAsFloats() + Vector2(0.5f, 0.5f);
		SpawnEntity(spawnPosition, 0.f, FACTION_ENEMY, ENTITY_TYPE_NPCTANK);
	}

	for (int i = 0; i < m_numIntialEnemyTurrets; i++)
	{
		int randomIndex = GetRandomIntLessThan(validTileCoords.size());
		IntVector2 tileCoords = validTileCoords[randomIndex];
		validTileCoords.erase(validTileCoords.begin() + randomIndex);
		Vector2 spawnPosition = tileCoords.GetAsFloats() + Vector2(0.5f, 0.5f);
		SpawnEntity(spawnPosition, 0.f, FACTION_ENEMY, ENTITY_TYPE_TURRET);
	}

	// Spawn some pickups
	for (int i = 0; i < 5; i++)
	{
		int randomIndex = GetRandomIntLessThan(validTileCoords.size());
		IntVector2 tileCoords = validTileCoords[randomIndex];
		validTileCoords.erase(validTileCoords.begin() + randomIndex);
		Vector2 spawnPosition = tileCoords.GetAsFloats() + Vector2(0.5f, 0.5f);
		SpawnEntity(spawnPosition, 0.f, FACTION_NEUTRAL, ENTITY_TYPE_PICKUP);
	}
}


//-----------------------------------------------------------------------------------------------
// Draws black lines on the screen to outline the grid
//
void Map::DrawGridLines() const
{
	g_theRenderer->SetDrawColor(Rgba::BLACK);

	// Drawing horizontal lines
	for (int yCoordinate = 0; yCoordinate < m_dimensions.y; yCoordinate++)
	{
		float yFloat = static_cast<float>(yCoordinate);
		g_theRenderer->DrawLine(Vector2(0.f, yFloat), Vector2(static_cast<float>(m_dimensions.x), yFloat));
	}

	// Drawing vertical lines
	for (int xCoordinate = 0; xCoordinate < m_dimensions.x; xCoordinate++)
	{
		float xFloat = static_cast<float>(xCoordinate);
		g_theRenderer->DrawLine(Vector2(xFloat, 0.f), Vector2(xFloat, static_cast<float>(m_dimensions.y)));
	}

	// Set the color back to white
	g_theRenderer->SetDrawColor(Rgba::WHITE);
}
