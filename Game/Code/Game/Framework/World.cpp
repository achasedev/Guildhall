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
#include "Game/Entity/Components/PhysicsComponent.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Assets/AssetDB.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Utility/HeatMap.hpp"
#include "Engine/Core/Time/ProfileLogScoped.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Core/Utility/ErrorWarningAssert.hpp"

#define DYNAMIC_COLLISION_MAX_ITERATION_COUNT (20)
#define MAX_COLLISION_LAYERS (3)

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

bool					IsThereTeamException(Entity* first, Entity* second);
bool					CheckEntityCollision(Entity* first, Entity* second);

BoundOverlapResult_t	PerformBroadphaseCheck(Entity* first, Entity* second);
VoxelOverlapResult_t	PerformNarrowPhaseCheck(Entity* first, Entity* second, const BoundOverlapResult_t& r);

void					CalculateCollisionCorrection(Entity* first, Entity* second, const VoxelOverlapResult_t& r);
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
	m_isQuitting = true;
	if (m_heatMapThread.joinable())
	{
		m_heatMapThread.join();
	}

	// Delete all maps
	delete m_navMapInUse.m_navigationMap;
	m_navMapInUse.m_navigationMap = nullptr;
	delete m_navMapInUse.m_costMap;
	m_navMapInUse.m_costMap = nullptr;

	delete m_intermediateMap.m_navigationMap;
	m_intermediateMap.m_navigationMap = nullptr;
	delete m_intermediateMap.m_costMap;
	m_intermediateMap.m_costMap = nullptr;

	delete m_backBufferMap.m_navigationMap;
	m_backBufferMap.m_navigationMap = nullptr;
	delete m_backBufferMap.m_costMap;
	m_backBufferMap.m_costMap = nullptr;
}


//-----------------------------------------------------------------------------------------------
// Initializes the grid and any other setup
//
void World::Inititalize()
{
	m_dimensions = IntVector3(256, 64, 256);

	m_groundElevation = 5;

	for (int i = 0; i < 5; ++i)
	{
		Entity* entity = new Entity(EntityDefinition::GetDefinition("Wall"));
		entity->SetPosition(Vector3(GetRandomFloatInRange(20.f, 200.f), (float) m_groundElevation, GetRandomFloatInRange(20.f, 200.f)));
		entity->SetOrientation(90.f);
		m_entities.push_back(entity);
	}

	//InitializeHeatMaps();
}


//-----------------------------------------------------------------------------------------------
// Cleans up the world's entities and state to be used again in another world
//
void World::CleanUp()
{
	m_dimensions = IntVector3(-1, -1, -1);
	m_groundElevation = 0;

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

	CleanUpHeatMaps();
}

#include "Engine/Input/InputSystem.hpp"
//-----------------------------------------------------------------------------------------------
// Update
//
void World::Update()
{
	if (InputSystem::GetInstance()->WasKeyJustPressed('F'))
	{
		m_drawCollision = !m_drawCollision;
	}

	if (InputSystem::GetInstance()->WasKeyJustPressed('G'))
	{
		m_drawHeatmap = !m_drawHeatmap;
	}

	PROFILE_LOG_SCOPE_FUNCTION();

	// "Thinking" and other general updating (animation)
	static bool updateEntities = true;

	if (InputSystem::GetInstance()->WasKeyJustPressed('K'))
	{
		updateEntities = !updateEntities;
	}

	if (updateEntities)
	{
		UpdateEntities();
		ApplyPhysicsStep();
	}
	UpdateParticles();

	// Moving the entities (Forward Euler)

	// Collision
	CheckDynamicEntityCollisions();
	CheckStaticEntityCollisions();

	// Clean Up
	DeleteMarkedEntities();

	// Navigation
	HeatMapUpdate_Main();
}	


//-----------------------------------------------------------------------------------------------
// Render
//
void World::Render()
{
	PROFILE_LOG_SCOPE_FUNCTION();

	VoxelGrid* grid = Game::GetVoxelGrid();

	// Color in the ground
	HeatMap* mapToUse = (m_drawHeatmap ? m_navMapInUse.m_navigationMap : nullptr);
	grid->DrawGround(m_groundElevation, mapToUse);

	// Color in static geometry
	DrawStaticEntitiesToGrid();

	// Color in each entity (shouldn't overlap, this is after physics step)
	DrawDynamicEntitiesToGrid();

	// Color in the particles
	DrawParticlesToGrid();
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
// Adds the given particle to the world
//
void World::AddParticle(Particle* particle)
{
	m_particles.push_back(particle);
	particle->OnSpawn();
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
	Vector2 topRight = bottomLeft + Vector2(entity->GetDimensions().xz()) + Vector2::ONES;

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
// Sets the entity to be on the ground at their current map position
//
void World::SnapEntityToGround(Entity* entity)
{
	if (IsEntityOnMap(entity))
	{
		Vector3 position = entity->GetPosition();
		position.y = (float)m_groundElevation;
		entity->SetPosition(position);

		PhysicsComponent* comp = entity->GetPhysicsComponent();
		if (comp != nullptr)
		{
			comp->ZeroYVelocity();
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Returns all entities that are within the given radius from the position
//
std::vector<const Entity*> World::GetEnemiesWithinDistance(const Vector3& position, float radius) const
{
	std::vector<const Entity*> entitiesInRange;

	int numEntities = (int)m_entities.size();
	float radiusSquared = (radius * radius);

	for (int index = 0; index < numEntities; ++index)
	{
		Entity* currEntity = m_entities[index];

		if (currEntity->GetTeam() != ENTITY_TEAM_ENEMY) { continue; }

		Vector3 entityPosition = currEntity->GetPosition();

		float distanceSquared = (position - entityPosition).GetLengthSquared();

		if (distanceSquared <= radiusSquared)
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
// Returns the navigation map of the world (one currently in use)
//
HeatMap* World::GetNavMap() const
{
	return m_navMapInUse.m_navigationMap;
}


//-----------------------------------------------------------------------------------------------
// Returns the ground elevation of the world
//
unsigned int World::GetGroundElevation() const
{
	return m_groundElevation;
}


//-----------------------------------------------------------------------------------------------
// Returns the coordinate that contains the position (round to nearest int)
//
IntVector3 World::GetCoordsForPosition(const Vector3& position) const
{
	return IntVector3(RoundToNearestInt(position.x), RoundToNearestInt(position.y), RoundToNearestInt(position.z));
}


//-----------------------------------------------------------------------------------------------
// Returns the next position along the path
//
Vector3 World::GetNextPositionTowardsPlayer(const Vector3& currPosition) const
{
	IntVector2 currCoords = GetCoordsForPosition(currPosition).xz();
	IntVector2 neighborCoords = m_navMapInUse.m_navigationMap->GetMinNeighborCoords(currCoords);

	return Vector3((float) neighborCoords.x, (float) m_groundElevation, (float) neighborCoords.y);
}


//-----------------------------------------------------------------------------------------------
// Returns true if the position passed is within a static entity (on the 2D plane), false otherwise
//
bool World::IsPositionInStatic(const Vector3& position) const
{
	IntVector2 coords = GetCoordsForPosition(position).xz();
	return (m_navMapInUse.m_navigationMap->GetHeat(coords) >= NAV_STATIC_COST);
}


//-----------------------------------------------------------------------------------------------
// Returns whether an entity at start position could see something at the end position
//
bool World::HasLineOfSight(const Vector3& startPosition, const Vector3& endPosition) const
{
	Vector3 direction = (endPosition - startPosition);
	int stepCount = Ceiling(direction.NormalizeAndGetLength());

	for (int stepNumber = 1; stepNumber <= stepCount; ++stepNumber)
	{
		Vector3 currPosition = startPosition + direction * (float) stepNumber;
		
		if (IsPositionInStatic(currPosition))
		{
			return false;
		}
	}

	return true;
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

	return (entity->GetPosition().y <= (float)m_groundElevation);
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
// Checks for entities clipping into the ground, and if so fixes them
//
void World::CheckForGroundCollisions()
{
	for (int entityIndex = 0; entityIndex < (int)m_entities.size(); ++entityIndex)
	{
		Entity* entity = m_entities[entityIndex];

		Vector3 position = entity->GetPosition();
		if (position.y < (float)m_groundElevation)
		{
			SnapEntityToGround(entity);
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
		
		// Don't bother with an entity marked for delete
		if (dynamicEntity->GetPhysicsType() != PHYSICS_TYPE_DYNAMIC || dynamicEntity->IsMarkedForDelete()) { continue; }

		for (int staticIndex = 0; staticIndex < (int)m_entities.size(); ++staticIndex)
		{
			Entity* staticEntity = m_entities[staticIndex];

			if (staticEntity->GetPhysicsType() != PHYSICS_TYPE_STATIC || staticEntity->IsMarkedForDelete()) { continue; }

			// Do detection and fix here
			CheckEntityCollision(dynamicEntity, staticEntity);
		}
	}

	ApplyCollisionCorrections();
	CheckForGroundCollisions();

	// Check particles for ground collisions
	for (int particleIndex = 0; particleIndex < (int)m_particles.size(); ++particleIndex)
	{
		Particle* currParticle = m_particles[particleIndex];

		// Don't bother with particles marked for delete
		if (currParticle->IsMarkedForDelete()) { continue; }

		// Also check for clipping into the ground
		Vector3 position = currParticle->GetPosition();
		if (position.y < (float)m_groundElevation)
		{
			SnapEntityToGround(currParticle);
// 			position.y = (float)m_groundElevation;
// 			currParticle->SetPosition(position);
// 			currParticle->GetPhysicsComponent()->SetVelocity(Vector3::ZERO);
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Checks for all dynamic vs. dynamic collisions in the scene, and corrects them
//
void World::CheckDynamicEntityCollisions()
{

	for (unsigned int layer = 0; layer < MAX_COLLISION_LAYERS; ++layer)
	{
		CheckForGroundCollisions();

		bool collisionDetected = true;
		for (int iteration = 0; iteration < (int)DYNAMIC_COLLISION_MAX_ITERATION_COUNT; ++iteration)
		{
			collisionDetected = false;

			for (int firstIndex = 0; firstIndex < (int)m_entities.size(); ++firstIndex)
			{
				Entity* firstEntity = m_entities[firstIndex];

				// Don't bother with an entity not on this layer, or marked for delete
				if (firstEntity->GetPhysicsType() != PHYSICS_TYPE_DYNAMIC || firstEntity->GetCollisionDefinition().m_collisionLayer != layer || firstEntity->IsMarkedForDelete()) { continue; }

				for (int secondIndex = 0; secondIndex < (int)m_entities.size(); ++secondIndex)
				{
					if (firstIndex == secondIndex) { continue; }

					Entity* secondEntity = m_entities[secondIndex];

					if (secondEntity->GetPhysicsType() != PHYSICS_TYPE_DYNAMIC || secondEntity->GetCollisionDefinition().m_collisionLayer > layer || secondEntity->IsMarkedForDelete()) { continue; }

					// Do detection and fix here
					collisionDetected = collisionDetected || CheckEntityCollision(firstEntity, secondEntity);
				}
			}

			if (!collisionDetected)
			{
				break;
			}
		
			ApplyCollisionCorrections();
		}
	}

	// Check against particles
	//bool collisionDetected = false;

// 	for (int iteration = 0; iteration < (int)DYNAMIC_COLLISION_MAX_ITERATION_COUNT; ++iteration)
// 	{
// 		collisionDetected = false;
// 
// 		for (int entityIndex = 0; entityIndex < (int)m_entities.size(); ++entityIndex)
// 		{
// 			Entity* entity = m_entities[entityIndex];
// 
// 			// Don't bother with an entity marked for delete
// 			if (entity->GetPhysicsType() != PHYSICS_TYPE_DYNAMIC || entity->IsMarkedForDelete()) { continue; }
// 
// 			for (int particleIndex = 0; particleIndex < (int)m_particles.size(); ++particleIndex)
// 			{
// 				Particle* particle = m_particles[particleIndex];
// 
// 				if (particle->IsMarkedForDelete()) { continue; }
// 
// 				// Do detection and fix here
// 				collisionDetected = collisionDetected || CheckEntityCollision(entity, particle);
// 			}
// 		}
// 
// 		if (!collisionDetected)
// 		{
// 			break;
// 		}
// 	}
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
}


//-----------------------------------------------------------------------------------------------
// Updates the navigation cost map by flagging spaces occupied by static entities as non-traversable
//
void World::UpdateCostMap()
{
	m_backBufferMap.m_costMap->Clear(1.f);

	int numSections = (int) m_staticSectionsThread.size();

	for (int sectionIndex = 0; sectionIndex < numSections; ++sectionIndex)
	{
		StaticSection_t currSection = m_staticSectionsThread[sectionIndex];

		for (int xOffset = 0; xOffset < currSection.dimensions.x; ++xOffset)
		{
			for (int zOffset = 0; zOffset < currSection.dimensions.z; ++zOffset)
			{
				IntVector2 coord = (currSection.coordPosition.xz() + IntVector2(xOffset, zOffset));

				if (m_backBufferMap.m_costMap->AreCoordsValid(coord))
				{
					m_backBufferMap.m_costMap->SetHeat(coord, NAV_STATIC_COST);
				}
			}
		}
	}

	// Cellular automata pass
	int neighborhoodSize = 8;
	bool currCoordsDone = false;
	for (unsigned int index = 0; index < m_backBufferMap.m_costMap->GetCellCount(); ++index)
	{
		// No need to update cells that are occupied by static entities
		if (m_backBufferMap.m_costMap->GetHeat(index) >= NAV_STATIC_COST) { continue; }

		currCoordsDone = false;
		IntVector2 currCoords = m_backBufferMap.m_costMap->GetCoordsForIndex(index);

		for (int y = -neighborhoodSize; y <= neighborhoodSize; ++y)
		{
			for (int x = -neighborhoodSize; x <= neighborhoodSize; ++x)
			{
				IntVector2 neighborCoords = currCoords + IntVector2(x, y);

				if (m_backBufferMap.m_costMap->AreCoordsValid(neighborCoords) && m_backBufferMap.m_costMap->GetHeat(neighborCoords) >= NAV_STATIC_COST)
				{
					m_backBufferMap.m_costMap->AddHeat(currCoords, 5.f);
					currCoordsDone = true;
					break;
				}
			}

			if (currCoordsDone)
			{
				break;
			}
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Deletes all entities marked for delete this frame, except for the players
//
void World::DeleteMarkedEntities()
{
	// Check for players before going into the dynamic list
	// Players will set themselves back to not marked, preventing deletion
// 	Player** players = Game::GetPlayers();
// 
// 	for (int i = 0; i < MAX_PLAYERS; ++i)
// 	{
// 		if (players[i] != nullptr && players[i]->IsMarkedForDelete())
// 		{
// 			players[i]->OnDeath();
// 		}
// 	}

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
void World::DrawStaticEntitiesToGrid()
{
	PROFILE_LOG_SCOPE_FUNCTION();

	VoxelGrid* grid = Game::GetVoxelGrid();

	int numEntities = (int)m_entities.size();

	for (int entityIndex = 0; entityIndex < numEntities; ++entityIndex)
	{
		if (m_entities[entityIndex]->GetPhysicsType() == PHYSICS_TYPE_STATIC && !m_entities[entityIndex]->IsMarkedForDelete())
		{
			if (m_drawCollision)
			{
				grid->DebugDrawEntityCollision(m_entities[entityIndex]);
			}
			else
			{
				grid->DrawEntity(m_entities[entityIndex]);
			}
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Draws all dynamic entities into the grid
//
void World::DrawDynamicEntitiesToGrid()
{
	PROFILE_LOG_SCOPE_FUNCTION();

	VoxelGrid* grid = Game::GetVoxelGrid();

	int numEntities = (int)m_entities.size();

	for (int entityIndex = 0; entityIndex < numEntities; ++entityIndex)
	{
		if (m_entities[entityIndex]->GetPhysicsType() == PHYSICS_TYPE_DYNAMIC && !m_entities[entityIndex]->IsMarkedForDelete())
		{
			if (m_drawCollision)
			{
				grid->DebugDrawEntityCollision(m_entities[entityIndex]);
			}
			else
			{
				grid->DrawEntity(m_entities[entityIndex]);
			}
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Draws the particles to the voxel grid
//
void World::DrawParticlesToGrid()
{
	VoxelGrid* grid = Game::GetVoxelGrid();

	int numParticles = (int)m_particles.size();

	for (int i = 0; i < numParticles; ++i)
	{
		if (!m_particles[i]->IsMarkedForDelete())
		{
			grid->DrawEntity(m_particles[i]);
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Creates all heatmaps that will be used for AI navigation
//
void World::InitializeHeatMaps()
{
	IntVector2 mapDimensions = m_dimensions.xz();

	m_navMapInUse.m_navigationMap = new HeatMap(mapDimensions, NAV_STATIC_COST);
	m_intermediateMap.m_navigationMap = new HeatMap(mapDimensions, NAV_STATIC_COST);
	m_backBufferMap.m_navigationMap = new HeatMap(mapDimensions, NAV_STATIC_COST);

	m_navMapInUse.m_costMap = new HeatMap(mapDimensions, 1.0f);
	m_intermediateMap.m_costMap = new HeatMap(mapDimensions, 1.0f);
	m_backBufferMap.m_costMap = new HeatMap(mapDimensions, 1.0f);

	// Start up the thread
	m_heatMapThread = std::thread(&World::HeatMapUpdate_Thread, this);
}


//-----------------------------------------------------------------------------------------------
// Gives the map thread updated static section and player positions, and swaps the heatmap in 
// use to the intermediate buffer if it's ready
//
void World::HeatMapUpdate_Main()
{
	PROFILE_LOG_SCOPE_FUNCTION();

	m_mapSwapLock.lock();

	// Update the static section data
	m_staticSectionsMain.clear();

	int numEntities = (int) m_entities.size();

	for (int entityIndex = 0; entityIndex < numEntities; ++entityIndex)
	{
		if (m_entities[entityIndex]->GetPhysicsType() != PHYSICS_TYPE_STATIC) { continue; }

		IntVector3 coords = m_entities[entityIndex]->GetCoordinatePosition();
		IntVector3 dimensions = m_entities[entityIndex]->GetDimensions();

		m_staticSectionsMain.push_back(StaticSection_t(coords, dimensions));
	}

	// Update the player seeds
	m_playerSeeds.clear();

	Player** players = Game::GetPlayers();

	for (int playerIndex = 0; playerIndex < MAX_PLAYERS; ++playerIndex)
	{
		if (Game::IsPlayerAlive(playerIndex))
		{
			m_playerSeeds.push_back(players[playerIndex]->GetCoordinatePosition());
		}
	}

	// Lastly check for a map update
	if (m_swapReady)
	{
		HeatMapSet_t temp = m_navMapInUse;
		m_navMapInUse = m_intermediateMap;
		m_intermediateMap = temp;
		m_swapReady = false;
	}

	m_mapSwapLock.unlock();
}


//-----------------------------------------------------------------------------------------------
// Updates the cost and heat maps used for AI navigation
//
void World::HeatMapUpdate_Thread()
{
	while (!m_isQuitting)
	{
		m_backBufferMap.m_navigationMap->Clear(NAV_STATIC_COST);

		// Get the lock to read the static sections
		m_mapSwapLock.lock_shared();

		// Update our list of positions
		m_staticSectionsThread.clear();

		int numSections = (int) m_staticSectionsMain.size();
		for (int sectionIndex = 0; sectionIndex < numSections; ++sectionIndex)
		{
			m_staticSectionsThread.push_back(m_staticSectionsMain[sectionIndex]);
		}

		// Also grab the player seeds and seed the navigation map
		int numSeeds = (int)m_playerSeeds.size();

		for (int seedIndex = 0; seedIndex < numSeeds; ++seedIndex)
		{
			m_backBufferMap.m_navigationMap->Seed(0.f, (m_playerSeeds[seedIndex].xz()));
		}

		// Unlock the lock
		m_mapSwapLock.unlock_shared();

		// Update the back cost map
		UpdateCostMap();

		// Solve the navigation map
		m_backBufferMap.m_navigationMap->SolveMapUpToDistance(NAV_STATIC_COST + 1.f, m_backBufferMap.m_costMap);

		// Update the intermediate map
		m_mapSwapLock.lock();

		HeatMapSet_t temp = m_backBufferMap;
		m_backBufferMap = m_intermediateMap;
		m_intermediateMap = temp;
		m_swapReady = true;

		m_mapSwapLock.unlock();
	}
}


//-----------------------------------------------------------------------------------------------
// Joins the heat map thread and deletes all existing heat maps
//
void World::CleanUpHeatMaps()
{
	m_isQuitting = true;
	m_heatMapThread.join();

	// Delete all maps
	delete m_navMapInUse.m_navigationMap;
	m_navMapInUse.m_navigationMap = nullptr;
	delete m_navMapInUse.m_costMap;
	m_navMapInUse.m_costMap = nullptr;

	delete m_intermediateMap.m_navigationMap;
	m_intermediateMap.m_navigationMap = nullptr;
	delete m_intermediateMap.m_costMap;
	m_intermediateMap.m_costMap = nullptr;

	delete m_backBufferMap.m_navigationMap;
	m_backBufferMap.m_navigationMap = nullptr;
	delete m_backBufferMap.m_costMap;
	m_backBufferMap.m_costMap = nullptr;

	m_staticSectionsMain.clear();
	m_staticSectionsThread.clear();
	m_swapReady = false;
	m_isQuitting = false;
}


//- C FUNCTION ----------------------------------------------------------------------------------------------
// Checks if the two entities pass the team exception check, and should continue checking for
// collisions. Returns true if there should be collision checks, false otherwise
//
bool IsThereTeamException(Entity* first, Entity* second)
{
	CollisionDefinition_t firstDef = first->GetCollisionDefinition();
	CollisionDefinition_t secondDef = second->GetCollisionDefinition();
	eEntityTeam firstTeam = first->GetTeam();
	eEntityTeam secondTeam = second->GetTeam();

	bool teamsMatch = (firstTeam == secondTeam);

	// Same checks
	if (firstDef.m_teamException == COLLISION_TEAM_EXCEPTION_SAME && teamsMatch) { return false; }
	if (secondDef.m_teamException == COLLISION_TEAM_EXCEPTION_SAME && teamsMatch) { return false; }

	// Different checks
	if (firstDef.m_teamException == COLLISION_TEAM_EXCEPTION_DIFFERENT && !teamsMatch) { return false; }
	if (secondDef.m_teamException == COLLISION_TEAM_EXCEPTION_DIFFERENT && !teamsMatch) { return false; }

	// Either both are always, or we don't meet the exception criteria
	return true;
}


//- C FUNCTION ----------------------------------------------------------------------------------------------
// Checks if the two given entities are colliding, and pushes them out if so
//
bool CheckEntityCollision(Entity* first, Entity* second)
{
	// First check for team exceptions
	if (!IsThereTeamException(first, second))
	{
		return false;
	}

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
		CalculateCollisionCorrection(first, second, voxelResult);

		first->OnCollision(second);
		second->OnCollision(first);

		first->OnVoxelCollision(voxelResult.firstHitCoords);
		second->OnVoxelCollision(voxelResult.secondHitCoords);
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

	IntVector3 firstDimensions = first->GetDimensions();
	IntVector3 secondDimensions = second->GetDimensions();

	Vector3 firstTopRight = firstPosition + Vector3(firstDimensions);
	Vector3 secondTopRight = secondPosition + Vector3(secondDimensions);

	AABB3 firstBounds = AABB3(firstPosition, firstTopRight);
	AABB3 secondBounds = AABB3(secondPosition, secondTopRight);

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

	// Calculate the X Overlap in voxels
	r.xOverlapf = MinFloat(firstBounds.maxs.x - secondBounds.mins.x, secondBounds.maxs.x - firstBounds.mins.x);
	r.xOverlapi = Ceiling(r.xOverlapf);
	r.xOverlapi = ClampInt(r.xOverlapi, 0, MinInt(firstDimensions.x, secondDimensions.x));

	ASSERT_OR_DIE(r.xOverlapi <= firstDimensions.x && r.xOverlapi <= secondDimensions.x, "Error: xOverlap too large");

	// Calculate the Y Overlap in voxels
	r.yOverlapf = MinFloat(firstBounds.maxs.y - secondBounds.mins.y, secondBounds.maxs.y - firstBounds.mins.y);
	r.yOverlapi = Ceiling(r.yOverlapf);
	r.yOverlapi = ClampInt(r.yOverlapi, 0, MinInt(firstDimensions.y, secondDimensions.y));

	ASSERT_OR_DIE(r.yOverlapi <= firstDimensions.y && r.yOverlapi <= secondDimensions.y, "Error: yOverlap too large");


	// Calculate the Z Overlap in voxels
	r.zOverlapf = MinFloat(firstBounds.maxs.z - secondBounds.mins.z, secondBounds.maxs.z - firstBounds.mins.z);
	r.zOverlapi = Ceiling(r.zOverlapf);
	r.zOverlapi = ClampInt(r.zOverlapi, 0, MinInt(firstDimensions.z, secondDimensions.z));

	ASSERT_OR_DIE(r.zOverlapi <= firstDimensions.z && r.zOverlapi <= secondDimensions.z, "Error: zOverlap too large");

	return r;
}


//- C FUNCTION ----------------------------------------------------------------------------------------------
// Checks if any voxels between the two entity's overlap space collide, and returns true if so
//
VoxelOverlapResult_t PerformNarrowPhaseCheck(Entity* first, Entity* second, const BoundOverlapResult_t& r)
{
	IntVector3 firstDimensions = first->GetDimensions();
	IntVector3 secondDimensions = second->GetDimensions();

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
	const VoxelTexture* firstTexture = first->GetTextureForRender();
	const VoxelTexture* secondTexture = second->GetTextureForRender();

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
			uint32_t firstFlags = firstTexture->GetCollisionByteForRow(yIndex + firstYOffset, zIndex + firstZOffset);
			uint32_t secondFlags = secondTexture->GetCollisionByteForRow(yIndex + secondYOffset, zIndex + secondZOffset);

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
					}
				}
			}

			if (firstFlags != 0 || secondFlags != 0)
			{
				minY = MinInt(minY, yIndex);
				minZ = MinInt(minZ, zIndex);
				maxY = MaxInt(maxY, yIndex);
				maxZ = MaxInt(maxZ, zIndex);
			}

			for (int xIndex = 0; xIndex < r.xOverlapi; ++xIndex)
			{
				if ((firstFlags & (TEXTURE_LEFTMOST_COLLISION_BIT >> xIndex)) != 0 || (secondFlags & (TEXTURE_LEFTMOST_COLLISION_BIT >> xIndex)) != 0)
				{
					minX = MinInt(minX, xIndex);
					maxX = MaxInt(maxX, xIndex);
				}
			}
		}
	}

	collisionResult.xOverlapi = MinInt(r.xOverlapi - minX, maxX + 1);
	collisionResult.yOverlapi = MinInt(r.yOverlapi - minY, maxY + 1);
	collisionResult.zOverlapi = MinInt(r.zOverlapi - minZ, maxZ + 1);

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

	if (collisionResult.xOverlapf > 1.0f) { collisionResult.xOverlapf -= 1.0f; }
	if (collisionResult.yOverlapf > 1.0f) { collisionResult.yOverlapf -= 1.0f; }
	if (collisionResult.zOverlapf > 1.0f) { collisionResult.zOverlapf -= 1.0f; }

	return collisionResult;
}


//- C FUNCTION ----------------------------------------------------------------------------------------------
// Applies a corrective collision offset to both entities so they are no longer overlapping
//
void CalculateCollisionCorrection(Entity* first, Entity* second, const VoxelOverlapResult_t& r)
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

	if (shouldCorrect)
	{
		// Apply the correction
		first->AddCollisionCorrection(firstScalar * finalCorrection);
		second->AddCollisionCorrection(-secondScalar * finalCorrection);

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
	const VoxelTexture* texture = entity->GetTextureForRender();
	Vector3 entityPosition = entity->GetBottomCenterPosition();

	unsigned int voxelCount = texture->GetVoxelCount();
	for (unsigned int i = 0; i < voxelCount; ++i)
	{
		Rgba color = texture->GetColorAtIndex(i);

		if (color.a != 0)
		{
			Vector3 voxelPosition = entity->GetPositionForLocalIndex(i);

			Vector3 velocity = (voxelPosition - entityPosition).GetNormalized();
			float speed = (20.f * GetRandomFloatInRange(0.f, 1.0f)) + 30.f;
			velocity *= speed;

			Particle* particle = new Particle(color, GetRandomFloatInRange(1.0f, 4.0f), voxelPosition, velocity);

			AddParticle(particle);
		}
	}
}
