/************************************************************************/
/* File: VoxelGrid.cpp
/* Author: Andrew Chase
/* Date: September 15th, 2018
/* Description: Implementation of the world class
/************************************************************************/
#include "Game/Entity/Player.hpp"
#include "Game/Framework/Game.hpp"
#include "Game/Entity/Particle.hpp"
#include "Game/Framework/World.hpp"
#include "Game/Framework/VoxelFont.hpp"
#include "Game/Framework/VoxelGrid.hpp"
#include "Game/Framework/GameCamera.hpp"
#include "Game/Framework/VoxelTerrain.hpp"
#include "Game/Framework/CampaignStage.hpp"
#include "Game/Entity/CharacterSelectVolume.hpp"
#include "Game/Entity/Components/PhysicsComponent.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Core/Image.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Core/LogSystem.hpp"
#include "Engine/Assets/AssetDB.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Utility/HeatMap.hpp"
#include "Engine/Core/Time/ProfileLogScoped.hpp"
#include "Engine/Core/Utility/ErrorWarningAssert.hpp"

#define DYNAMIC_COLLISION_MAX_ITERATION_COUNT (10)
#define MAX_COLLISION_LAYERS (3)
//#define PARTICLES_COLLIDE

enum eOverlapCase
{
	FIRST_ON_MIN,
	FIRST_ON_MAX,
	FIRST_INSIDE,
	FIRST_ENCAPSULATE
};

struct BoundOverlapResult_t
{
	BoundOverlapResult_t(bool result)
		: overlapOccurred(result) {}

	bool overlapOccurred = false;

	int xOverlapi;
	int yOverlapi;
	int zOverlapi;

	float xOverlapf;
	float yOverlapf;
	float zOverlapf;

	eOverlapCase xCase;
	eOverlapCase yCase;
	eOverlapCase zCase;

	AABB3 firstBounds;
	AABB3 secondBounds;
};

struct VoxelOverlapResult_t
{
	bool overlapOccurred = false;

	int xOverlapi;
	int yOverlapi;
	int zOverlapi;

	float xOverlapf;
	float yOverlapf;
	float zOverlapf;

	std::vector<IntVector3> firstHitCoords;
	std::vector<IntVector3> secondHitCoords;
};

//bool					IsThereTeamException(Entity* first, Entity* second);
bool					DoEntitiesCollide(Entity* firstEntity, Entity* secondEntity);
bool					CheckEntityCollision(Entity* first, Entity* second);

BoundOverlapResult_t	PerformBroadphaseCheck(Entity* first, Entity* second);
VoxelOverlapResult_t	PerformNarrowPhaseCheck(Entity* first, Entity* second, const BoundOverlapResult_t& r);

void					CalculateAndApplyCollisionCorrection(Entity* first, Entity* second, const VoxelOverlapResult_t& r);
void					CalculateMassScalars(Entity* first, Entity* second, float& out_firstScalar, float& out_secondScalar);


//-----------------------------------------------------------------------------------------------
// Constructor
//
World::World()
{
}


//-----------------------------------------------------------------------------------------------
// Destructor
//
World::~World()
{
	CleanUp();
}


//-----------------------------------------------------------------------------------------------
// Sets up the world to be ready for the main/character select menu
//
void World::InitializeForMenu()
{
	if (m_terrain != nullptr)
	{
		delete m_terrain;
		m_terrain = nullptr;
	}
}


//-----------------------------------------------------------------------------------------------
// Initializes the grid and any other setup
//
void World::InititalizeForStage(CampaignStage* stage)
{
	if (m_terrain != nullptr)
	{
		delete m_terrain;
	}

	// Clean up entities
	for (int i = 0; i < (int)m_entities.size(); ++i)
	{
		if (!m_entities[i]->IsPlayer())
		{
			delete m_entities[i];
		}
	}

	m_entities.clear();

	// Clean up particles
	for (int i = 0; i < (int)m_particles.size(); ++i)
	{
		delete m_particles[i];
	}

	m_particles.clear();

	// Initialize terrain and static entities on terrain
	InitializeTerrain(stage->m_mapName);
	
	// Add in the entities from the stage
	int numEntities = (int) stage->m_initialStatics.size();

	for (int entityIndex = 0; entityIndex < numEntities; ++entityIndex)
	{
		InitialStaticSpawn_t& spawn = stage->m_initialStatics[entityIndex];

		Entity* entity = new CharacterSelectVolume(spawn.definition);

		entity->SetPosition(spawn.position);
		entity->SetOrientation(spawn.orientation);

		AddEntity(entity);
	}

	// Add in the players
	Player** players = Game::GetPlayers();
	for (int i = 0; i < MAX_PLAYERS; ++i)
	{
		if (players[i] != nullptr)
		{
			AddEntity(players[i]);
		}
	}

	// Set the transition edge
	m_enterEdge = stage->m_edgeToEnter;
}


//-----------------------------------------------------------------------------------------------
// Cleans up the world's entities and state to be used again in another world
//
void World::CleanUp()
{
	for (int i = 0; i < (int)m_entities.size(); ++i)
	{
		if (dynamic_cast<Player*>(m_entities[i]) == nullptr)
		{
			delete m_entities[i];
		}
	}

	m_entities.clear();

	for (int i = 0; i < (int)m_particles.size(); ++i)
	{
		delete m_particles[i];
	}

	m_particles.clear();

	if (m_terrain != nullptr)
	{
		delete m_terrain;
		m_terrain = nullptr;
	}
}

#include "Engine/Input/InputSystem.hpp"
//-----------------------------------------------------------------------------------------------
// Update
//
void World::Update()
{
	if (InputSystem::GetInstance()->WasKeyJustPressed('I'))
	{
		m_drawCollisions = !m_drawCollisions;
	}

	PROFILE_LOG_SCOPE_FUNCTION();

	// "Thinking" and other general updating (animation)
	UpdateEntities();
	UpdateParticles();

	// Moving the entities (Forward Euler)
	ApplyPhysicsStep();

	// Collision
	CheckDynamicEntityCollisions();
	CheckStaticEntityCollisions();
	CheckMapCollisions();

	// Clean Up
	DeleteMarkedEntities();
}	


//-----------------------------------------------------------------------------------------------
// Render
//
void World::DrawToGrid()
{
	PROFILE_LOG_SCOPE_FUNCTION();

	DrawToGridWithOffset(IntVector3::ZERO);
}


//-----------------------------------------------------------------------------------------------
// Draws the world to the grid, treating offset as the origin
//
void World::DrawToGridWithOffset(const IntVector3& offset)
{
	VoxelGrid* grid = Game::GetVoxelGrid();

	// Color in the ground, if there is one
	if (m_terrain != nullptr)
	{
		grid->DrawTerrain(m_terrain, offset);
	}

	// Color in static geometry
	DrawStaticEntitiesToGrid(offset);

	// Color in each entity (shouldn't overlap, this is after physics step)
	DrawDynamicEntitiesToGrid(offset);

	// Color in the particles
	DrawParticlesToGrid(offset);
}


//-----------------------------------------------------------------------------------------------
// Adds the entity to the world
//
void World::AddEntity(Entity* entity)
{
	m_entities.push_back(entity);
	entity->OnSpawn();
}


//-----------------------------------------------------------------------------------------------
// Removes the entity from the world, without destroying it
//
void World::RemoveEntity(Entity* entity)
{
	int numEntities = (int)m_entities.size();

	for (int i = 0; i < numEntities; ++i)
	{
		if (m_entities[i] == entity)
		{
			m_entities.erase(m_entities.begin() + i);
			break;
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Adds the given particle to the world
//
void World::AddParticle(Particle* particle)
{
	m_particles.push_back(particle);
	particle->OnSpawn();
}


//-----------------------------------------------------------------------------------------------
// Creates an explosion at the given coord and radius, destroying terrain and hitting entities 
//
void World::ApplyExplosion(const IntVector3& coord, eEntityTeam team, int damage /*= 0*/, 
	float radius /*= 0.f*/, float impulseMagnitude /*= 0.f*/, Entity* hitEntity /*=nullptr*/)
{
	DestroyTerrain(coord, radius, impulseMagnitude);

	// Apply damage and knockback to entities within the radius
	std::vector<Entity*> entities = GetEntitiesThatOverlapSphere(Vector3(coord), radius);

	for (int entityIndex = 0; entityIndex < (int)entities.size(); ++entityIndex)
	{
		Entity* currEntity = entities[entityIndex];

		if (currEntity->GetTeam() == team) { continue; }

		if (currEntity->IsDynamic())
		{
			Vector3 direction = (currEntity->GetCenterPosition() - Vector3(coord)).GetNormalized();
			Vector3 impulse = direction * impulseMagnitude;
			currEntity->TakeDamage(damage, impulse);
		}
		else if (currEntity->IsDestructible())
		{
			// Find all voxels in the entity that were affected
			std::vector<IntVector3> hitCoords;
			IntVector3 entityDimensions = currEntity->GetOrientedDimensions();
			float radiusSquared = radius * radius;

			for (int y = 0; y < entityDimensions.y; ++y)
			{
				for (int z = 0; z < entityDimensions.z; ++z)
				{
					for (int x = 0; x < entityDimensions.x; ++x)
					{
						IntVector3 currCoord = IntVector3(x, y, z);
						Vector3 currPos = currEntity->GetPositionForLocalCoords(currCoord);
						float distanceSquared = (currPos - Vector3(coord)).GetLengthSquared();

						if (distanceSquared < radiusSquared)
						{
							hitCoords.push_back(currCoord);
						}
					}
				}
			}

			// Call the callback
			currEntity->OnVoxelCollision(nullptr, hitCoords);
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Sets the voxel at the given coord to the given color in the terrain
//
void World::AddVoxelToTerrain(const IntVector3& coord, const Rgba& color)
{
	m_terrain->AddVoxel(coord, color);
}


//-----------------------------------------------------------------------------------------------
// Sets whether the world should block edge collisions; used to turn off edge collisions during
// transitions, then to re-enable for normal play
//
void World::SetBlockEdgeCollision(bool shouldBlock)
{
	m_blockEdgeCollisions = shouldBlock;
}


//-----------------------------------------------------------------------------------------------
// Blows up the given entity
//
void World::ParticalizeAllEntities()
{
	for (int entityIndex = (int) m_entities.size() - 1; entityIndex >= 0; --entityIndex)
	{
		Entity* entity = m_entities[entityIndex];

		if (dynamic_cast<Player*>(entity) == nullptr)
		{
			ParticalizeEntity(entity);
			delete entity;
			m_entities.erase(m_entities.begin() + entityIndex);
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Returns whether the entity is completely on the map
//
bool World::IsEntityOnMap(const Entity* entity) const
{
	Vector2 bottomLeft = entity->GetPosition().xz();
	Vector2 topRight = bottomLeft + Vector2(entity->GetOrientedDimensions().xz()) + Vector2::ONES;

	if (bottomLeft.x < 0.f || bottomLeft.y < 0.f)
	{
		return false;
	}

	if (topRight.x > (float) m_dimensions.x || topRight.y > (float) m_dimensions.z)
	{
		return false;
	}

	return true;
}


//-----------------------------------------------------------------------------------------------
// Returns whether the given 2D coordinates are on the map
//
bool World::AreCoordsOnMap(const IntVector2& coords) const
{
	bool onX = coords.x >= 0 && coords.x < m_dimensions.x;
	bool onZ = coords.y >= 0 && coords.y < m_dimensions.z;

	return (onX && onZ);
}


//-----------------------------------------------------------------------------------------------
// Returns whether the given coords are inside the voxel world (3D)
//
bool World::AreCoordsInWorld(const IntVector3& coords) const
{
	bool onX = coords.x >= 0 && coords.x < m_dimensions.x;
	bool onY = coords.y >= 0 && coords.y < m_dimensions.y;
	bool onZ = coords.z >= 0 && coords.z < m_dimensions.z;

	return (onX && onY && onZ);
}


//-----------------------------------------------------------------------------------------------
// Returns all entities that are within the given radius from the position
//
std::vector<Entity*> World::GetEntitiesThatOverlapSphere(const Vector3& position, float radius) const
{
	std::vector<Entity*> entitiesInRange;

	int numEntities = (int)m_entities.size();

	for (int index = 0; index < numEntities; ++index)
	{
		Entity* currEntity = m_entities[index];

		AABB3 entityBounds = currEntity->GetWorldBounds();

		if (DoesBoxSphereOverlap(entityBounds, position, radius))
		{
			entitiesInRange.push_back(currEntity);
		}
	}

	return entitiesInRange;
}


//-----------------------------------------------------------------------------------------------
// Returns the voxel dimensions of the world
//
IntVector3 World::GetDimensions() const
{
	return m_dimensions;
}


//-----------------------------------------------------------------------------------------------
// Returns the height of the map at the given coordinate location
//
int World::GetGroundElevationAtCoord(const IntVector2& coord) const
{
	return m_terrain->GetHeightAtCoords(coord);
}


//-----------------------------------------------------------------------------------------------
// Returns the max elevation of the terrain
//
int World::GetCurrentMaxHeightOfTerrain() const
{
	int max = -1;

	for (int x = 0; x < m_dimensions.x; ++x)
	{
		for (int z = 0; z < m_dimensions.z; ++z)
		{
			max = MaxInt(max, (int) GetGroundElevationAtCoord(IntVector2(x, z)));
		}
	}

	return max;
}


//-----------------------------------------------------------------------------------------------
// Returns the edge that the players enter on for this map
//
eTransitionEdge World::GetDirectionToEnter() const
{
	return m_enterEdge;
}


//-----------------------------------------------------------------------------------------------
// Returns the coordinate that contains the position (round to nearest int)
//
IntVector3 World::GetCoordsForPosition(const Vector3& position) const
{
	return IntVector3(RoundToNearestInt(position.x), RoundToNearestInt(position.y), RoundToNearestInt(position.z));
}


//-----------------------------------------------------------------------------------------------
// Returns true if the entity is standing on (or in) the ground
//
bool World::IsEntityOnGround(const Entity* entity) const
{
	if (!IsEntityOnMap(entity))
	{
		return false;
	}

	return (entity->GetPosition().y <= GetMapHeightForEntity(entity));
}


//-----------------------------------------------------------------------------------------------
// Draws the given text particalized to the grid, for effects
//
void World::ParticalizeVoxelText(const std::string& text, const IntVector3& referenceStart, const VoxelFontDraw_t& options)
{
	IntVector3 textDimensions = options.font->GetTextDimensions(text);
	textDimensions.x *= options.scale.x;
	textDimensions.y *= options.scale.y;
	textDimensions.z *= options.scale.z;

	// Add in the border
	textDimensions.x += 2 * options.borderThickness;
	textDimensions.y += 2 * options.borderThickness;

	IntVector3 forward = IntVector3(CrossProduct(Vector3(options.right), Vector3(options.up)));
	IntVector3 startWorldCoord = referenceStart;
	startWorldCoord -= options.right * (int)((float)textDimensions.x * options.alignment.x);
	startWorldCoord -= options.up * (int)((float)textDimensions.y * options.alignment.y);
	startWorldCoord -= forward * (int)((float)textDimensions.z * options.alignment.z);

	IntVector3 glyphDimensions = options.font->GetGlyphDimensions();

	for (int zOff = 0; zOff < textDimensions.z; ++zOff)
	{
		for (int yOff = 0; yOff < textDimensions.y; ++yOff)
		{
			for (int xOff = 0; xOff < textDimensions.x; ++xOff)
			{
				int charIndex = ((xOff - options.borderThickness) / options.scale.x) / glyphDimensions.x;
				int xOffset = ((xOff - options.borderThickness) / options.scale.x) % glyphDimensions.x;
				int yOffset = (yOff - options.borderThickness) / options.scale.y;

				IntVector3 worldOffset = options.right * xOff + options.up * yOff + forward * zOff;

				IntVector3 finalCoords = startWorldCoord + worldOffset;
				Vector3 finalPosition = Vector3(finalCoords) + Vector3(0.5f, 0.5f, 0.5f);
				Vector3 velocity = Vector3(GetRandomFloatInRange(-1.f, 1.f), 1.f, GetRandomFloatInRange(-1.f, 1.f)) * 50.f;
				Rgba baseColor = options.font->GetColorForGlyphPixel(text[charIndex], IntVector2(xOffset, yOffset));

				Particle* particle = new Particle(baseColor, 1.0f, finalPosition, velocity, false);
				AddParticle(particle);
			}
		}
	}
}

#include "Engine/Core/DeveloperConsole/DevConsole.hpp"
//-----------------------------------------------------------------------------------------------
// Sets the terrain and initial entities given the name of the map
//
void World::InitializeTerrain(const std::string mapName)
{
	m_terrain = VoxelTerrain::GetTerrainClone(mapName);

	const std::vector<EntitySpawn_t> spawns = m_terrain->GetInitialEntities();

	for (int i = 0; i < (int)spawns.size(); ++i)
	{
		Entity* entity = new Entity(spawns[i].definition);
		entity->SetPosition(spawns[i].position);

		entity->SetOrientation(spawns[i].orientation);

		AddEntity(entity);
	}
}


//-----------------------------------------------------------------------------------------------
// Updates all entities in the game that aren't particles
//
void World::UpdateEntities()
{
	PROFILE_LOG_SCOPE_FUNCTION();

	int numEntities = (int)m_entities.size();

	for (int i = 0; i < numEntities; ++i)
	{
		if (!m_entities[i]->IsMarkedForDelete())
		{
			m_entities[i]->Update();
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Updates all particles in the game
//
void World::UpdateParticles()
{
	PROFILE_LOG_SCOPE_FUNCTION();

	int numParticles = (int)m_particles.size();

	for (int i = 0; i < numParticles; ++i)
	{
		if (!m_particles[i]->IsMarkedForDelete())
		{
			m_particles[i]->Update();
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Applies the physics step to all dynamic entities
//
void World::ApplyPhysicsStep()
{
	int numDynamics = (int)m_entities.size();

	for (int i = 0; i < numDynamics; ++i)
	{
		Entity* currDynamic = m_entities[i];

		if (currDynamic->GetPhysicsType() == PHYSICS_TYPE_DYNAMIC && !currDynamic->IsMarkedForDelete() && currDynamic->IsPhysicsEnabled())
		{
			m_entities[i]->GetPhysicsComponent()->ApplyPhysicsStep();
		}
	}

	// Apply it to particles too
	int numParticles = (int)m_particles.size();

	for (int i = 0; i < numParticles; ++i)
	{
		if (!m_particles[i]->IsMarkedForDelete() && m_particles[i]->IsPhysicsEnabled())
		{
			m_particles[i]->GetPhysicsComponent()->ApplyPhysicsStep();
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Returns the height of the map under the given entity
//
int World::GetMapHeightForEntity(const Entity* entity) const
{
	IntVector3 coordPosition = entity->GetCoordinatePosition();
	IntVector2 dimensions = entity->GetOrientedDimensions().xz();

	return GetMapHeightForBounds(coordPosition, dimensions);
}


//-----------------------------------------------------------------------------------------------
// Returns the max terrain height at the given position and dimensions
//
int World::GetMapHeightForBounds(const IntVector3& coordPosition, const IntVector2& dimensions) const
{
	if (m_terrain == nullptr)
	{
		return 0;
	}

	int maxHeight = 0;

	// Check all voxels on the entity's bottom, ensuring that it's not clipping into the ground
	for (int z = 0; z < dimensions.y; ++z)
	{
		for (int x = 0; x < dimensions.x; ++x)
		{
			IntVector3 currPos = coordPosition + IntVector3(x, 0, z);
			if (AreCoordsOnMap(currPos.xz()))
			{
				int currHeight = m_terrain->GetHeightAtCoords(currPos.xz());

				// Update the max height over this area
				maxHeight = MaxInt(maxHeight, currHeight);
			}
		}
	}

	return maxHeight;
}


//-----------------------------------------------------------------------------------------------
// Checks for entities clipping into the ground, and if so fixes them
//
void World::CheckEntityForGroundCollision(Entity* entity)
{
	Vector3 position = entity->GetPosition();
	IntVector3 coordPosition = entity->GetCoordinatePosition();

	int mapHeight = GetMapHeightForEntity(entity);
	bool touchingGround = (mapHeight >= position.y);

	if (touchingGround)
	{
		// If we are through some ground, snap to it
		if (mapHeight > 0)
		{
			// Call the event *before* we correct, to get the exact hit location
			entity->OnGroundCollision();

			// Then snap to map height
			position.y = mapHeight;
			entity->SetPosition(position);

			PhysicsComponent* comp = entity->GetPhysicsComponent();
			if (comp != nullptr)
			{
				comp->ZeroYVelocity();
			}
		}
		else
		{
			// Fell in a pit - let them fall until they're hidden, then kill them
			IntVector3 dimensions = entity->GetOrientedDimensions();
			if (position.y <= -(float)dimensions.y + 4)
			{
				entity->TakeDamage(9999999999);
			}
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Destroys the terrain at coord and within a given radius
//
void World::DestroyTerrain(const IntVector3& hitCoordinate, float radius /*= 0.f*/, float impulseMagnitude /*= 0.f*/)
{
	if (!AreCoordsInWorld(hitCoordinate))
	{
		return;
	}

	// For the ground I hit, blow it away! (anything within radius)
	int xStart = hitCoordinate.x - RoundToNearestInt(radius);
	int yStart = hitCoordinate.y - RoundToNearestInt(radius);
	int zStart = hitCoordinate.z - RoundToNearestInt(radius);

	int xEnd = hitCoordinate.x + RoundToNearestInt(radius);
	int yEnd = hitCoordinate.y + RoundToNearestInt(radius);
	int zEnd = hitCoordinate.z + RoundToNearestInt(radius);

	float radiusSquared = radius * radius;

	for (int y = yStart; y < yEnd; ++y)
	{
		for (int z = zStart; z < zEnd; ++z)
		{
			for (int x = xStart; x < xEnd; ++x)
			{
				IntVector3 currCoord = IntVector3(x, y, z);
				if (!AreCoordsInWorld(currCoord))
				{
					continue;
				}

				float distanceSquared = (Vector3(hitCoordinate) - Vector3(currCoord)).GetLengthSquared();
				int heightAtCurrCoord = m_terrain->GetHeightAtCoords(currCoord.xz());

				if (distanceSquared <= radiusSquared)
				{
					Rgba color = m_terrain->RemoveVoxel(currCoord);

					if (CheckRandomChance(0.1f))
					{
						Vector3 velocity = impulseMagnitude * Vector3(GetRandomFloatInRange(-1.f, 1.f), 1.f, GetRandomFloatInRange(-1.f, 1.f));

						Particle* particle = new Particle(color, 2.0f, Vector3(currCoord), velocity, false);
						AddParticle(particle);
					}
				}
				else if (y > hitCoordinate.y && y < heightAtCurrCoord)
				{
					Rgba color = m_terrain->GetColorAtCoords(IntVector3(currCoord.x, y, currCoord.z));

					Particle* particle = new Particle(color, 2.0f, Vector3(currCoord), Vector3::ZERO, false);
					AddParticle(particle);
				}
			}
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Checks for all dynamic vs. static collisions in the scene, and corrects them
//
void World::CheckStaticEntityCollisions()
{
	for (int dynamicIndex = 0; dynamicIndex < (int)m_entities.size(); ++dynamicIndex)
	{
		Entity* dynamicEntity = m_entities[dynamicIndex];
		
		// Find an entity that is dynamic and not marked for delete
		if (!dynamicEntity->IsDynamic() || dynamicEntity->IsMarkedForDelete()) 
		{ 
			continue; 
		}

		for (int staticIndex = 0; staticIndex < (int)m_entities.size(); ++staticIndex)
		{
			Entity* staticEntity = m_entities[staticIndex];

			// Only check against static entities that aren't marked for delete
			if (staticEntity->IsDynamic() || staticEntity->IsMarkedForDelete()) 
			{ 
				continue; 
			}

			// Do detection and fix here
			CheckEntityCollision(dynamicEntity, staticEntity);
		}
	}

	// Apply the cached off corrections
	ApplyCollisionCorrections();
}


//-----------------------------------------------------------------------------------------------
// Checks to ensure dynamic entities stay on the map during normal play, and fixes ground clipping
//
void World::CheckMapCollisions()
{
	CheckGroundCollisions();
	
	if (m_blockEdgeCollisions)
	{
		CheckEdgeCollisions();
	}
}


//-----------------------------------------------------------------------------------------------
// Checks if any entities are clipping through the ground, and if so snaps them to the correct height
//
void World::CheckGroundCollisions()
{
	// Ensure the entities are above ground
	for (int i = 0; i < (int)m_entities.size(); ++i)
	{
		Entity* dynamicEntity = m_entities[i];

		// Find an entity that is dynamic and not marked for delete
		if (!dynamicEntity->IsDynamic() || dynamicEntity->IsMarkedForDelete())
		{
			continue;
		}

		CheckEntityForGroundCollision(dynamicEntity);
	}

	// Check particles for ground collisions
	for (int particleIndex = 0; particleIndex < (int)m_particles.size(); ++particleIndex)
	{
		Particle* currParticle = m_particles[particleIndex];

		// Don't bother with particles marked for delete
		if (currParticle->IsMarkedForDelete()) { continue; }

		CheckEntityForGroundCollision(currParticle);
	}
}


//-----------------------------------------------------------------------------------------------
// Checks if any dynamic entities are clipping off the map, and if so corrects them
//
void World::CheckEdgeCollisions()
{
	for (int entityIndex = 0; entityIndex < (int)m_entities.size(); ++entityIndex)
	{
		Entity* entity = m_entities[entityIndex];

		AABB3 worldBounds = entity->GetWorldBounds();
		Vector3 bottomLeft = worldBounds.mins;
		Vector3 topRight = worldBounds.maxs;

		Vector3 correction = Vector3::ZERO;

		// Check x
		float leftCorrection = -bottomLeft.x;
		if (leftCorrection < 0.f)
		{
			leftCorrection = 0.f;
		}

		float rightCorrection = (float)m_dimensions.x - topRight.x;
		if (rightCorrection > 0.f)
		{
			rightCorrection = 0.f;
		}

		correction.x = (AbsoluteValue(leftCorrection) > AbsoluteValue(rightCorrection) ? leftCorrection : rightCorrection);

		// Check z
		float bottomCorrection = -bottomLeft.z;
		if (bottomCorrection < 0.f)
		{
			bottomCorrection = 0.f;
		}

		float topCorrection = (float)m_dimensions.z - topRight.z;
		if (topCorrection > 0.f)
		{
			topCorrection = 0.f;
		}

		correction.z = (AbsoluteValue(bottomCorrection) > AbsoluteValue(topCorrection) ? bottomCorrection : topCorrection);

		// Directly apply correction
		entity->AddPositionOffset(correction);

		PhysicsComponent* component = entity->GetPhysicsComponent();

		if (component != nullptr)
		{
			if (correction.x != 0.f)
			{
				component->ZeroXVelocity();
			}

			if (correction.z != 0.f)
			{
				component->ZeroZVelocity();
			}
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Checks for all dynamic vs. dynamic collisions in the scene, and corrects them
//
void World::CheckDynamicEntityCollisions()
{
	bool collisionDetectedOverall = true;
	for (int iteration = 0; iteration < (int)DYNAMIC_COLLISION_MAX_ITERATION_COUNT; ++iteration)
	{
		collisionDetectedOverall = false;

		for (int firstIndex = 0; firstIndex < (int)m_entities.size(); ++firstIndex)
		{
			Entity* firstEntity = m_entities[firstIndex];

			// Don't bother with the entity if it isn't dynamic or is marked for delete
			if (!firstEntity->IsDynamic() || firstEntity->IsMarkedForDelete())
			{
				continue;
			}

			for (int secondIndex = firstIndex + 1; secondIndex < (int)m_entities.size(); ++secondIndex)
			{
				if (firstIndex == secondIndex) { continue; }

				Entity* secondEntity = m_entities[secondIndex];

				if (!secondEntity->IsDynamic() || secondEntity->IsMarkedForDelete())
				{
					continue;
				}

				// Make sure the entities are on colliding collision layers
				if (DoEntitiesCollide(firstEntity, secondEntity))
				{
					// Do detection and cache off correction here
					bool collisionDetected = CheckEntityCollision(firstEntity, secondEntity) || collisionDetectedOverall;
					collisionDetectedOverall = collisionDetected || collisionDetectedOverall;
				}
			}
		}

		if (!collisionDetectedOverall)
		{
			break;
		}

		ApplyCollisionCorrections();
	}

	// Check against particles
#ifdef PARTICLES_COLLIDE

	for (int iteration = 0; iteration < (int)DYNAMIC_COLLISION_MAX_ITERATION_COUNT; ++iteration)
	{
		collisionDetectedOverall = false;

		for (int entityIndex = 0; entityIndex < (int)m_entities.size(); ++entityIndex)
		{
			Entity* entity = m_entities[entityIndex];

			// Only check against dynamic entities
			if (!entity->IsDynamic() || entity->IsMarkedForDelete())
			{ 
				continue; 
			}

			for (int particleIndex = 0; particleIndex < (int)m_particles.size(); ++particleIndex)
			{
				Particle* particle = m_particles[particleIndex];

				if (particle->IsMarkedForDelete()) 
				{ 
					continue; 
				}

				// Do detection and fix here
				bool collisionDetected = CheckEntityCollision(entity, particle);
				collisionDetectedOverall = collisionDetectedOverall || collisionDetected;
			}
		}

		if (!collisionDetectedOverall)
		{
			break;
		}

		ApplyCollisionCorrections();
	}
#endif
}


//-----------------------------------------------------------------------------------------------
// Applies all collision corrections that entities have accumulated
//
void World::ApplyCollisionCorrections()
{
	int entityCount = (int)m_entities.size();
	
	for (int i = 0; i < entityCount; ++i)
	{
		if (!m_entities[i]->IsMarkedForDelete())
		{
			m_entities[i]->ApplyCollisionCorrection();
		}
	}

	int particleCount = (int)m_particles.size();
	for (int i = 0; i < particleCount; ++i)
	{
		if (!m_particles[i]->IsMarkedForDelete())
		{
			m_particles[i]->ApplyCollisionCorrection();
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Deletes all entities marked for delete this frame, except for the players
//
void World::DeleteMarkedEntities()
{
	// Then check entities
	for (int i = (int)m_entities.size() - 1; i >= 0; --i)
	{
		Entity* entity = m_entities[i];

		if (entity->IsMarkedForDelete() && !entity->IsPlayer())
		{
			// OnDeath should have been called, and once finished should mark for delete
			delete entity;

			if (i < (int)m_entities.size() - 1)
			{
				m_entities[i] = m_entities.back();
			}

			m_entities.pop_back();
		}
	}

	// Check particles
	int numParticles = (int)m_particles.size();

	for (int i = numParticles - 1; i >= 0; --i)
	{
		if (m_particles[i]->IsMarkedForDelete())
		{
			m_particles[i]->OnDeath();
			delete m_particles[i];

			if (i < (int)m_particles.size() - 1)
			{
				m_particles[i] = m_particles.back();
			}

			m_particles.pop_back();
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Draws all static entities into the voxel grid
//
void World::DrawStaticEntitiesToGrid(const IntVector3& offset)
{
	PROFILE_LOG_SCOPE_FUNCTION();

	VoxelGrid* grid = Game::GetVoxelGrid();

	int numEntities = (int)m_entities.size();

	for (int entityIndex = 0; entityIndex < numEntities; ++entityIndex)
	{
		if (m_entities[entityIndex]->GetPhysicsType() == PHYSICS_TYPE_STATIC && !m_entities[entityIndex]->IsMarkedForDelete())
		{
			if (m_drawCollisions)
			{
				grid->DrawEntityCollision(m_entities[entityIndex], offset);
			}
			else
			{
				VoxelDrawOptions_t options;
				options.castsShadows = true;
				options.receivesShadows = true;

				grid->DrawEntity(m_entities[entityIndex], offset, options);
			}
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Draws all dynamic entities into the grid
//
void World::DrawDynamicEntitiesToGrid(const IntVector3& offset)
{
	PROFILE_LOG_SCOPE_FUNCTION();

	VoxelGrid* grid = Game::GetVoxelGrid();

	int numEntities = (int)m_entities.size();

	for (int entityIndex = 0; entityIndex < numEntities; ++entityIndex)
	{
		if (m_entities[entityIndex]->GetPhysicsType() == PHYSICS_TYPE_DYNAMIC && !m_entities[entityIndex]->IsMarkedForDelete())
		{
			Rgba whiteReplacement = Rgba::WHITE;
			if (m_entities[entityIndex]->IsPlayer())
			{
				Player* player = dynamic_cast<Player*>(m_entities[entityIndex]);
				whiteReplacement = player->GetPlayerColor();
			}

			if (m_drawCollisions)
			{
				grid->DrawEntityCollision(m_entities[entityIndex], offset);
			}
			else
			{
				VoxelDrawOptions_t options;
				options.castsShadows = true;
				options.receivesShadows = true;
				options.whiteReplacement = whiteReplacement;

				grid->DrawEntity(m_entities[entityIndex], offset, options);
			}
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Draws the particles to the voxel grid
//
void World::DrawParticlesToGrid(const IntVector3& offset)
{
	VoxelGrid* grid = Game::GetVoxelGrid();

	int numParticles = (int)m_particles.size();

	for (int i = 0; i < numParticles; ++i)
	{
		if (!m_particles[i]->IsMarkedForDelete())
		{
			grid->DrawEntity(m_particles[i], offset);
		}
	}
}


//- C FUNCTION ----------------------------------------------------------------------------------------------
// Returns true if the two given collision layers collide with eachother
//
bool DoEntitiesCollide(Entity* firstEntity, Entity* secondEntity)
{
	eCollisionLayer firstLayer = firstEntity->GetCollisionLayer();
	eCollisionLayer secondLayer = secondEntity->GetCollisionLayer();

	return (firstLayer & secondLayer) != 0;
}


//- C FUNCTION ----------------------------------------------------------------------------------------------
// Checks if the two given entities are colliding, and pushes them out if so
//
bool CheckEntityCollision(Entity* first, Entity* second)
{
	CollisionDefinition_t firstDef = first->GetCollisionDefinition();
	CollisionDefinition_t secondDef = second->GetCollisionDefinition();

	BoundOverlapResult_t boundsResult = PerformBroadphaseCheck(first, second);

	if (!boundsResult.overlapOccurred)
	{
		return false;
	}

	// Now Do Narrow Phase
	VoxelOverlapResult_t voxelResult = PerformNarrowPhaseCheck(first, second, boundsResult);

	if (voxelResult.overlapOccurred)
	{
		// Apply corrections and call collision callbacks
		CalculateAndApplyCollisionCorrection(first, second, voxelResult);

		first->OnEntityCollision(second);
		second->OnEntityCollision(first);

		first->OnVoxelCollision(second, voxelResult.firstHitCoords);
		second->OnVoxelCollision(first, voxelResult.secondHitCoords);
	}

	return voxelResult.overlapOccurred;
}


//- C FUNCTION ----------------------------------------------------------------------------------------------
// Checks if the AABB3 bounds of the two entities overlap each other, and returns the result
//
BoundOverlapResult_t PerformBroadphaseCheck(Entity* first, Entity* second)
{
	// Get state of the entities
	Vector3 firstPosition = first->GetPosition();
	Vector3 secondPosition = second->GetPosition();

	AABB3 firstBounds = first->GetWorldBounds();
	AABB3 secondBounds = second->GetWorldBounds();

	// Check if the bounds overlap, and if not can early out
	if (!DoAABB3sOverlap(firstBounds, secondBounds))
	{
		return BoundOverlapResult_t(false);
	}

	// Bounds overlap, so return details about the collision
	BoundOverlapResult_t r = BoundOverlapResult_t(true);


	r.firstBounds = firstBounds;
	r.secondBounds = secondBounds;

	//-----Set up the relative position results for the first entity-----
	// Important for determining offsets into each entity's collision matrix on
	// all 3 dimensions

	// X
	if (firstBounds.maxs.x < secondBounds.maxs.x)
	{
		if (firstBounds.mins.x > secondBounds.mins.x)
		{
			r.xCase = FIRST_INSIDE;
		}
		else
		{
			r.xCase = FIRST_ON_MIN;
		}
	}
	else
	{
		if (firstBounds.mins.x > secondBounds.mins.x)
		{
			r.xCase = FIRST_ON_MAX;
		}
		else
		{
			r.xCase = FIRST_ENCAPSULATE;
		}
	}

	// Y
	if (firstBounds.maxs.y < secondBounds.maxs.y)
	{
		if (firstBounds.mins.y > secondBounds.mins.y)
		{
			r.yCase = FIRST_INSIDE;
		}
		else
		{
			r.yCase = FIRST_ON_MIN;
		}
	}
	else
	{
		if (firstBounds.mins.y > secondBounds.mins.y)
		{
			r.yCase = FIRST_ON_MAX;
		}
		else
		{
			r.yCase = FIRST_ENCAPSULATE;
		}
	}

	// Z
	if (firstBounds.maxs.z < secondBounds.maxs.z)
	{
		if (firstBounds.mins.z > secondBounds.mins.z)
		{
			r.zCase = FIRST_INSIDE;
		}
		else
		{
			r.zCase = FIRST_ON_MIN;
		}
	}
	else
	{
		if (firstBounds.mins.z > secondBounds.mins.z)
		{
			r.zCase = FIRST_ON_MAX;
		}
		else
		{
			r.zCase = FIRST_ENCAPSULATE;
		}
	}

	IntVector3 firstDimensions = first->GetOrientedDimensions();
	IntVector3 secondDimensions = second->GetOrientedDimensions();

	// Calculate the X Overlap in voxels
	r.xOverlapf = MinFloat(firstBounds.maxs.x - secondBounds.mins.x, secondBounds.maxs.x - firstBounds.mins.x);
	r.xOverlapi = RoundToNearestInt(r.xOverlapf);
	r.xOverlapi = ClampInt(r.xOverlapi, 0, MinInt(firstDimensions.x, secondDimensions.x));

	ASSERT_OR_DIE(r.xOverlapi <= firstDimensions.x && r.xOverlapi <= secondDimensions.x, "Error: xOverlap too large");

	// Calculate the Y Overlap in voxels
	r.yOverlapf = MinFloat(firstBounds.maxs.y - secondBounds.mins.y, secondBounds.maxs.y - firstBounds.mins.y);
	r.yOverlapi = RoundToNearestInt(r.yOverlapf);
	r.yOverlapi = ClampInt(r.yOverlapi, 0, MinInt(firstDimensions.y, secondDimensions.y));

	ASSERT_OR_DIE(r.yOverlapi <= firstDimensions.y && r.yOverlapi <= secondDimensions.y, "Error: yOverlap too large");


	// Calculate the Z Overlap in voxels
	r.zOverlapf = MinFloat(firstBounds.maxs.z - secondBounds.mins.z, secondBounds.maxs.z - firstBounds.mins.z);
	r.zOverlapi = RoundToNearestInt(r.zOverlapf);
	r.zOverlapi = ClampInt(r.zOverlapi, 0, MinInt(firstDimensions.z, secondDimensions.z));

	ASSERT_OR_DIE(r.zOverlapi <= firstDimensions.z && r.zOverlapi <= secondDimensions.z, "Error: zOverlap too large");

	return r;
}


//- C FUNCTION ----------------------------------------------------------------------------------------------
// Checks if any voxels between the two entity's overlap space collide, and returns true if so
//
VoxelOverlapResult_t PerformNarrowPhaseCheck(Entity* first, Entity* second, const BoundOverlapResult_t& r)
{
	IntVector3 firstDimensions = first->GetOrientedDimensions();
	IntVector3 secondDimensions = second->GetOrientedDimensions();

	//-----Set up offsets based on the relative position of first-----
	int firstYOffset = 0;
	int firstZOffset = 0;
	int secondYOffset = 0;
	int secondZOffset = 0;

	switch (r.zCase)
	{
	case FIRST_ON_MIN:
		firstZOffset = (int)firstDimensions.z - r.zOverlapi;

		break;
	case FIRST_ON_MAX:
		secondZOffset = (int)secondDimensions.z - r.zOverlapi;

		break;
	case FIRST_INSIDE:
	{
		secondZOffset = (int)r.firstBounds.mins.z - (int)r.secondBounds.mins.z;
	}
	break;
	case FIRST_ENCAPSULATE:
	{
		firstZOffset = (int)r.secondBounds.mins.z - (int)r.firstBounds.mins.z;
	}

	break;
	}

	switch (r.yCase)
	{
	case FIRST_ON_MIN:
		firstYOffset = (int)firstDimensions.y - r.yOverlapi;

		break;
	case FIRST_ON_MAX:
		secondYOffset = (int)secondDimensions.y - r.yOverlapi;

		break;
	case FIRST_INSIDE:
	{
		secondYOffset = (int)r.firstBounds.mins.y - (int)r.secondBounds.mins.y;
	}


	break;
	case FIRST_ENCAPSULATE:
	{
		firstYOffset = (int)r.secondBounds.mins.y - (int)r.firstBounds.mins.y;
	}

	break;
	}

	// Get the textures
	const VoxelSprite* firstTexture = first->GetVoxelSprite();
	const VoxelSprite* secondTexture = second->GetVoxelSprite();

	int minX = 100;
	int minY = 100;
	int minZ = 100;
	int maxX = -1;
	int maxY = -1;
	int maxZ = -1;

	VoxelOverlapResult_t collisionResult;

	for (int yIndex = 0; yIndex < r.yOverlapi; ++yIndex)
	{
		for (int zIndex = 0; zIndex < r.zOverlapi; ++zIndex)
		{
			uint32_t firstFlags = firstTexture->GetCollisionByteForRow(yIndex + firstYOffset, zIndex + firstZOffset, first->GetOrientation());
			uint32_t secondFlags = secondTexture->GetCollisionByteForRow(yIndex + secondYOffset, zIndex + secondZOffset, second->GetOrientation());

			// Bitshift the flags based on the x overlaps
			switch (r.xCase)
			{
				case FIRST_ON_MIN: // First straddles the "left" edge of second
				{
					firstFlags = firstFlags << (firstDimensions.x - r.xOverlapi);
				}
				break;

				case FIRST_ON_MAX: // First straddles the "right" edge of second
				{
					secondFlags = secondFlags << (secondDimensions.x - r.xOverlapi);
				}
				break;

				case FIRST_INSIDE: // First is within second
				{
					int bitsOnLeft = RoundToNearestInt(r.firstBounds.mins.x - r.secondBounds.mins.x);

					ASSERT_OR_DIE(bitsOnLeft >= 0, "Error: BitsOnLeft was negative");

					// Make the mask
					uint32_t mask = 0;
					int index = bitsOnLeft;
					for (int i = 0; i < r.xOverlapi; ++i)
					{
						mask |= (TEXTURE_LEFTMOST_COLLISION_BIT >> index);
						++index;
					}

					// Isolate the bits in the overlap
					secondFlags = secondFlags & mask;
					secondFlags = secondFlags << bitsOnLeft;
				}
				break;

				case FIRST_ENCAPSULATE: // First encapsulates second
				{
					int bitsOnLeft = RoundToNearestInt(r.secondBounds.mins.x - r.firstBounds.mins.x);

					ASSERT_OR_DIE(bitsOnLeft >= 0, "Error: BitsOnLeft was negative");

					// Make the mask
					uint32_t mask = 0;
					int index = bitsOnLeft;
					for (int i = 0; i < r.xOverlapi; ++i)
					{
						mask |= (TEXTURE_LEFTMOST_COLLISION_BIT >> index);
						++index;
					}

					// Isolate the bits in the overlap
					firstFlags = firstFlags & mask;
					firstFlags = firstFlags << bitsOnLeft;
				}
				break;
			}

			// Actual check - if they share any bits, return true immediately
			// Make the mask
			uint32_t mask = 0;
			for (int i = 0; i < r.xOverlapi; ++i)
			{
				mask |= (TEXTURE_LEFTMOST_COLLISION_BIT >> i);
			}

			uint32_t flagResult = (firstFlags & secondFlags & mask);
			if (flagResult != 0)
			{
				collisionResult.overlapOccurred = true;

				for (int xIndex = 0; xIndex < r.xOverlapi; ++xIndex)
				{
					// Need to get the relative offsets into the collision for when each entity starts
					int firstXOffset = 0;
					int secondXOffset = 0;
					if ((flagResult & (TEXTURE_LEFTMOST_COLLISION_BIT >> xIndex)) != 0)
					{
						switch (r.xCase)
						{
						case FIRST_ON_MIN:
						{
							firstXOffset = (firstDimensions.x - r.xOverlapi);
						}
						break;

						case FIRST_ON_MAX:
						{
							secondXOffset = (secondDimensions.x - r.xOverlapi);
						}
						break;

						case FIRST_INSIDE:
						{
							secondXOffset = RoundToNearestInt(r.firstBounds.mins.x - r.secondBounds.mins.x);
						}
						break;

						case FIRST_ENCAPSULATE:
						{
							firstXOffset = RoundToNearestInt(r.secondBounds.mins.x - r.firstBounds.mins.x);
						}
						break;
						}

						IntVector3 firstCoords = IntVector3(xIndex + firstXOffset, yIndex + firstYOffset, zIndex + firstZOffset);
						IntVector3 secondCoords = IntVector3(xIndex + secondXOffset, yIndex + secondYOffset, zIndex + secondZOffset);

						collisionResult.firstHitCoords.push_back(firstCoords);
						collisionResult.secondHitCoords.push_back(secondCoords);

						minX = MinInt(minX, xIndex);
						maxX = MaxInt(maxX, xIndex);
					}
				}

				minY = MinInt(minY, yIndex);
				minZ = MinInt(minZ, zIndex);
				maxY = MaxInt(maxY, yIndex);
				maxZ = MaxInt(maxZ, zIndex);
			}
		}
	}

	collisionResult.xOverlapi = ClampInt(maxX - minX + 1, 0, 100);
	collisionResult.yOverlapi = ClampInt(maxY - minY + 1, 0, 100);	
	collisionResult.zOverlapi = ClampInt(maxZ - minZ + 1, 0, 100);

	collisionResult.xOverlapf = (float)collisionResult.xOverlapi;
	collisionResult.yOverlapf = (float)collisionResult.yOverlapi;
	collisionResult.zOverlapf = (float)collisionResult.zOverlapi;

	Vector3 firstPosition = first->GetPosition();
	Vector3 secondPosition = second->GetPosition();
	Vector3 firstCoord = Vector3(first->GetCoordinatePosition());
	Vector3 secondCoord = Vector3(second->GetCoordinatePosition());

	Vector3 firstError = firstCoord - firstPosition;
	Vector3 secondError = secondCoord - secondPosition;

	if (r.xCase == FIRST_ON_MIN)
	{
		collisionResult.xOverlapf -= firstError.x;
		collisionResult.xOverlapf += secondError.x;
	}
	else if (r.xCase == FIRST_ON_MAX)
	{
		collisionResult.xOverlapf += firstError.x;
		collisionResult.xOverlapf -= secondError.x;
	}

	if (r.yCase == FIRST_ON_MIN)
	{
		collisionResult.yOverlapf -= firstError.y;
		collisionResult.yOverlapf += secondError.y;
	}
	else if (r.yCase == FIRST_ON_MAX)
	{
		collisionResult.yOverlapf += firstError.y;
		collisionResult.yOverlapf -= secondError.y;
	}

	if (r.zCase == FIRST_ON_MIN)
	{
		collisionResult.zOverlapf -= firstError.z;
		collisionResult.zOverlapf += secondError.z;
	}
	else if (r.zCase == FIRST_ON_MAX)
	{
		collisionResult.zOverlapf += firstError.z;
		collisionResult.zOverlapf -= secondError.z;
	}

	return collisionResult;
}


//- C FUNCTION ----------------------------------------------------------------------------------------------
// Applies a corrective collision offset to both entities so they are no longer overlapping
//
void CalculateAndApplyCollisionCorrection(Entity* first, Entity* second, const VoxelOverlapResult_t& r)
{
	Vector3 firstPosition = first->GetPosition();
	Vector3 secondPosition = second->GetPosition();

	Vector3 diff = (firstPosition - secondPosition);
	Vector3 absDiff = Vector3(AbsoluteValue(diff.x), AbsoluteValue(diff.y), AbsoluteValue(diff.z));

	// Determine the correction offset by pushing across the least amount of overlap
	Vector3 finalCorrection;
	int direction = -1; // 0 for x, 1 for y, 2 for z
	if (r.xOverlapf < r.zOverlapf)
	{
		if (r.xOverlapf <= r.yOverlapf)
		{
			float sign = (diff.x < 0.f ? -1.f : 1.f);
			finalCorrection = Vector3(sign * (float)r.xOverlapf, 0.f, 0.f);
			direction = 0;
		}
		else
		{
			float sign = (diff.y < 0.f ? -1.f : 1.f);
			finalCorrection = Vector3(0.f, sign * (float)r.yOverlapf, 0.f);
			direction = 1;
		}
	}
	else
	{
		if (r.zOverlapf <= r.yOverlapf)
		{
			float sign = (diff.z < 0.f ? -1.f : 1.f);
			finalCorrection = Vector3(0.f, 0.f, sign * (float)r.zOverlapf);
			direction = 2;
		}
		else
		{
			float sign = (diff.y < 0.f ? -1.f : 1.f);
			finalCorrection = Vector3(0.f, sign * (float)r.yOverlapf, 0.f);
			direction = 1;
		}
	}

	// Get the scalars that determine how they will share the correction
	float firstScalar, secondScalar;
	CalculateMassScalars(first, second, firstScalar, secondScalar);

	bool shouldCorrect = (firstScalar > 0.f || secondScalar > 0.f);

	Vector3 firstFinalCorrection = firstScalar * finalCorrection;
	Vector3 secondFinalCorrection = -secondScalar * finalCorrection;
	Vector3 firstPosPreCorrection = first->GetPosition();
	Vector3 secondPosPreCorrection = second->GetPosition();

	if (shouldCorrect)
	{
		// Apply the correction
		first->AddCollisionCorrection(firstFinalCorrection);
		second->AddCollisionCorrection(secondFinalCorrection);

		ConsolePrintf("Collision applied");
		// Zero out velocities if colliding against static
		if (first->GetPhysicsType() == PHYSICS_TYPE_STATIC && second->GetPhysicsType() == PHYSICS_TYPE_DYNAMIC)
		{
			PhysicsComponent* secondComp = second->GetPhysicsComponent();

			if (secondComp != nullptr)
			{
				switch (direction)
				{
				case 0:
					secondComp->ZeroXVelocity();
					break;
				case 1:
					secondComp->ZeroYVelocity();
					break;
				case 2:
					secondComp->ZeroZVelocity();
					break;
				default:
					break;
				}
			}
		}

		if (first->GetPhysicsType() == PHYSICS_TYPE_DYNAMIC && second->GetPhysicsType() == PHYSICS_TYPE_STATIC)
		{
			PhysicsComponent* firstComp = first->GetPhysicsComponent();

			if (firstComp != nullptr)
			{
				switch (direction)
				{
				case 0:
					firstComp->ZeroXVelocity();
					break;
				case 1:
					firstComp->ZeroYVelocity();
					break;
				case 2:
					firstComp->ZeroZVelocity();
					break;
				default:
					break;
				}
			}
		}
	}

	// Log a bunch of info for checking
	if (first->IsPlayer() || second->IsPlayer() && first->GetPhysicsType() == PHYSICS_TYPE_STATIC || second->GetPhysicsType() == PHYSICS_TYPE_STATIC)
	{
		LogTaggedPrintf("GAME", "Collision hit");

		LogTaggedPrintf("Game", "Positions: (First: (%.2f, %.2f, %.2f) | (Second: (%.2f, %.2f, %.2f)", firstPosPreCorrection.x, firstPosPreCorrection.y, firstPosPreCorrection.z, secondPosPreCorrection.x, secondPosPreCorrection.y, secondPosPreCorrection.z);

		AABB3 firstBounds = first->GetWorldBounds();
		AABB3 secondBounds = second->GetWorldBounds();

		LogTaggedPrintf("GAME", "Bounds: (First: Mins(%.2f, %.2f, %.2f) Maxs(%.2f, %.2f, %.2f) | (Second: Mins(%.2f, %.2f, %.2f) Maxs(%.2f, %.2f, %.2f)", firstBounds.mins.x, firstBounds.mins.y, firstBounds.mins.z, firstBounds.maxs.x, firstBounds.maxs.y, firstBounds.maxs.z, secondBounds.mins.x, secondBounds.mins.y, secondBounds.mins.z, secondBounds.maxs.x, secondBounds.maxs.y, secondBounds.maxs.z);

		LogTaggedPrintf("GAME", "Overlaps Amounts (Int): (%i, %i, %i)", r.xOverlapi, r.yOverlapi, r.zOverlapi);
		LogTaggedPrintf("GAME", "Overlaps Amounts (Float): (%.2f, %.2f, %.2f)", r.xOverlapf, r.yOverlapf, r.zOverlapf);
		LogTaggedPrintf("GAME", "Direction chosen: %i", direction);

		LogTaggedPrintf("GAME", "First correction: Scalar = %.2f, Overall Correction = (%.2f, %.2f, %.2f), Final Applied =  (%.2f, %.2f, %.2f)", firstScalar, finalCorrection.x, finalCorrection.y, finalCorrection.z, firstFinalCorrection.x, firstFinalCorrection.y, firstFinalCorrection.z);
		LogTaggedPrintf("GAME", "Second correction: Scalar = %.2f, Overall Correction = (%.2f, %.2f, %.2f), Final Applied =  (%.2f, %.2f, %.2f)", secondScalar, finalCorrection.x, finalCorrection.y, finalCorrection.z, secondFinalCorrection.x, secondFinalCorrection.y, secondFinalCorrection.z);
	
		first->ApplyCollisionCorrection();
		second->ApplyCollisionCorrection();

		Vector3 firstPos = first->GetPosition();
		Vector3 secondPos = second->GetPosition();

		LogTaggedPrintf("GAME", "Pushed First to (%.2f, %.2f, %.2f)", firstPos.x, firstPos.y, firstPos.z);
		LogTaggedPrintf("GAME", "Pushed Second to (%.2f, %.2f, %.2f)", secondPos.x, secondPos.y, secondPos.z);
	}
}


//- C FUNCTION ----------------------------------------------------------------------------------------------
// Returns the two scalars to use during a collision correction, based on the entities' masses and collision
// definitions
//
void CalculateMassScalars(Entity* first, Entity* second, float& out_firstScalar, float& out_secondScalar)
{
	CollisionDefinition_t firstDef = first->GetCollisionDefinition();
	CollisionDefinition_t secondDef = second->GetCollisionDefinition();

	// If either one is flagged as ignoring the correction then do nothing at all
	if (firstDef.m_response == COLLISION_RESPONSE_IGNORE_CORRECTION || secondDef.m_response == COLLISION_RESPONSE_IGNORE_CORRECTION)
	{
		out_firstScalar = 0.f;
		out_secondScalar = 0.f;
		return;
	}

	float firstMass = first->GetMass();
	float secondMass = second->GetMass();
	float massSum = firstMass + secondMass;

	if (firstDef.m_response == COLLISION_RESPONSE_NO_CORRECTION)
	{
		if (secondDef.m_response == COLLISION_RESPONSE_NO_CORRECTION)
		{
			out_firstScalar = 0.f;
			out_secondScalar = 0.f;
		}
		else if (secondDef.m_response == COLLISION_RESPONSE_FULL_CORRECTION)
		{
			out_firstScalar = 0.f;
			out_secondScalar = 1.0f;
		}
		else if (secondDef.m_response == COLLISION_RESPONSE_SHARE_CORRECTION)
		{
			out_firstScalar = 0.f;
			out_secondScalar = 1.0f;
		}
	}
	else if (firstDef.m_response == COLLISION_RESPONSE_FULL_CORRECTION)
	{
		if (secondDef.m_response == COLLISION_RESPONSE_NO_CORRECTION)
		{
			out_firstScalar = 1.0f;
			out_secondScalar = 0.f;
		}
		else if (secondDef.m_response == COLLISION_RESPONSE_FULL_CORRECTION)
		{
			out_firstScalar = firstMass / massSum;
			out_secondScalar = 1.0f - out_firstScalar;
		}
		else if (secondDef.m_response == COLLISION_RESPONSE_SHARE_CORRECTION)
		{
			out_firstScalar = 1.0f;
			out_secondScalar = 0.f;
		}
	}
	else if (firstDef.m_response == COLLISION_RESPONSE_SHARE_CORRECTION)
	{
		if (secondDef.m_response == COLLISION_RESPONSE_NO_CORRECTION)
		{
			out_firstScalar = 1.0f;
			out_secondScalar = 0.f;
		}
		else if (secondDef.m_response == COLLISION_RESPONSE_FULL_CORRECTION)
		{
			out_firstScalar = 0.f;
			out_secondScalar = 1.0f;
		}
		else if (secondDef.m_response == COLLISION_RESPONSE_SHARE_CORRECTION)
		{
			out_firstScalar = firstMass / massSum;
			out_secondScalar = 1.0f - out_firstScalar;
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Creates a bunch of particles to represent the exploded entity
//
void World::ParticalizeEntity(Entity* entity)
{
	const VoxelSprite* texture = entity->GetVoxelSprite();
	Vector3 entityPosition = entity->GetBottomCenterPosition();

	unsigned int voxelCount = texture->GetVoxelCount();
	for (unsigned int i = 0; i < voxelCount; ++i)
	{
		Rgba color = texture->GetColorAtIndex(i);

		if (color.a != 0 && CheckRandomChance(0.25f))
		{
			Vector3 voxelPosition = entity->GetPositionForLocalIndex(i);

			Vector3 velocity = (voxelPosition - entityPosition).GetNormalized();
			float speed = (20.f * GetRandomFloatInRange(0.f, 1.0f)) + 30.f;
			velocity *= speed;

			Particle* particle = new Particle(color, 1000.f, voxelPosition, velocity);

			AddParticle(particle);
		}
	}
}
