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

#define DYNAMIC_COLLISION_MAX_ITERATION_COUNT (5)
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
	BoundOverlapResult_t(bool result = false)
		: overlapOccurred(result) {}

	bool overlapOccurred = false;

	int xOverlap;
	int yOverlap;
	int zOverlap;

	float xOverlapf;
	float yOverlapf;
	float zOverlapf;

	eOverlapCase xCase;
	eOverlapCase yCase;
	eOverlapCase zCase;

	AABB3 firstBounds;
	AABB3 secondBounds;
};

bool					CheckAndCorrectEntityCollision(Entity* first, Entity* second);

BoundOverlapResult_t	PerformBroadphaseCheck(Entity* first, Entity* second);
bool					PerformNarrowPhaseCheck(Entity* first, Entity* second, const BoundOverlapResult_t& r);

void					ApplyCollisionCorrection(Entity* first, Entity* second, const BoundOverlapResult_t& r);
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
	if (m_playerHeatmap != nullptr)
	{
		delete m_playerHeatmap;
		m_playerHeatmap = nullptr;
	}
}


//-----------------------------------------------------------------------------------------------
// Initializes the grid and any other setup
//
void World::Inititalize()
{
	m_dimensions = IntVector3(256, 64, 256);

	m_voxelGrid = new VoxelGrid();
	m_voxelGrid->Initialize(m_dimensions);

	m_groundElevation = 5;

	IntVector2 mapDimensions = m_dimensions.xz();
	mapDimensions.x /= NAV_DIMENSION_FACTOR;
	mapDimensions.y /= NAV_DIMENSION_FACTOR;

	m_playerHeatmap = new HeatMap(mapDimensions, NAV_STATIC_COST);
	m_costsMap = new HeatMap(mapDimensions, 1.0f);

	IntVector2 targetCoords = IntVector2(240, 240);
	targetCoords.x /= NAV_DIMENSION_FACTOR;
	targetCoords.y /= NAV_DIMENSION_FACTOR;

	UpdateCostMap();
	
	m_playerHeatmap->SetHeat(targetCoords, 0.f);
	m_playerHeatmap->SolveMapUpToDistance(NAV_STATIC_COST + 1.f, m_costsMap);

	for (int i = 0; i < 1; ++i)
	{
		Entity* entity = new Entity(EntityDefinition::GetDefinition("Window"));
		entity->SetPosition(Vector3(GetRandomFloatInRange(20.f, 200.f), (float) m_groundElevation, GetRandomFloatInRange(20.f, 200.f)));
		entity->SetOrientation(90.f);
		m_entities.push_back(entity);
	}
}


//-----------------------------------------------------------------------------------------------
// Cleans up the world's entities and state to be used again in another world
//
void World::CleanUp()
{
	m_dimensions = IntVector3(-1, -1, -1);
	m_groundElevation = 0;
	
	delete m_voxelGrid;
	m_voxelGrid = nullptr;

	delete m_playerHeatmap;
	m_playerHeatmap = nullptr;

	delete m_costsMap;
	m_costsMap = nullptr;

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

	PROFILE_LOG_SCOPE_FUNCTION();

	// "Thinking" and other general updating (animation)
	UpdateEntities();
	UpdateParticles();

	// Moving the entities (Forward Euler)
	ApplyPhysicsStep();

	// Collision
	CheckDynamicEntityCollisions();
	CheckStaticEntityCollisions();

	// Clean Up
	DeleteMarkedEntities();

	// Navigation
	//UpdateCostMap();
	//UpdatePlayerHeatmap();
}	


//-----------------------------------------------------------------------------------------------
// Render
//
void World::Render()
{
	PROFILE_LOG_SCOPE_FUNCTION();

	// Clear grid
	m_voxelGrid->Clear();

	// Color in the ground
	m_voxelGrid->DrawGround(m_groundElevation);

	// Color in static geometry
	DrawStaticEntitiesToGrid();

	// Color in each entity (shouldn't overlap, this is after physics step)
	DrawDynamicEntitiesToGrid();

	// Color in the particles
	DrawParticlesToGrid();

	// Rebuild the mesh and draw it to screen
	m_voxelGrid->BuildMeshAndDraw();
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
// Returns the voxel dimensions of the world
//
IntVector3 World::GetDimensions() const
{
	return m_dimensions;
}


//-----------------------------------------------------------------------------------------------
// Returns the navigation map of the world
//
HeatMap* World::GetNavMap() const
{
	return m_playerHeatmap;
}


//-----------------------------------------------------------------------------------------------
// Returns the ground elevation of the world
//
unsigned int World::GetGroundElevation() const
{
	return m_groundElevation;
}


//-----------------------------------------------------------------------------------------------
// Returns the next position along the path
//
Vector3 World::GetNextPositionTowardsPlayer(const Vector3& currPosition) const
{
	IntVector2 currCoords = IntVector2(currPosition.x, currPosition.z);

	currCoords.x /= NAV_DIMENSION_FACTOR;
	currCoords.y /= NAV_DIMENSION_FACTOR;

	IntVector2 neighborCoords = m_playerHeatmap->GetMinNeighborCoords(currCoords);

	neighborCoords.x *= NAV_DIMENSION_FACTOR;
	neighborCoords.y *= NAV_DIMENSION_FACTOR;

	return Vector3((float) neighborCoords.x + (NAV_DIMENSION_FACTOR * .5f), (float) m_groundElevation, (float) neighborCoords.y + (NAV_DIMENSION_FACTOR * .5f));
}


//-----------------------------------------------------------------------------------------------
// Returns true if the position passed is within a static entity (on the 2D plane), false otherwise
//
bool World::IsPositionInStatic(const Vector3& position) const
{
	IntVector2 coords = IntVector2(position.xz()) / NAV_DIMENSION_FACTOR;
	return (m_playerHeatmap->GetHeat(coords) >= NAV_STATIC_COST);
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

		if (currDynamic->GetPhysicsType() == PHYSICS_TYPE_DYNAMIC && !currDynamic->IsMarkedForDelete())
		{
			m_entities[i]->GetPhysicsComponent()->ApplyPhysicsStep();
		}
	}

	// Apply it to particles too
	int numParticles = (int)m_particles.size();

	for (int i = 0; i < numParticles; ++i)
	{
		if (!m_particles[i]->IsMarkedForDelete() && m_particles[i]->ShouldApplyPhysics())
		{
			m_particles[i]->GetPhysicsComponent()->ApplyPhysicsStep();

			// Optimization - set the particle to not keep simulating if it already hit the ground
			if (IsEntityOnGround(m_particles[i]))
			{
				m_particles[i]->SetApplyPhysics(false);
			}
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

		// Also check for clipping into the ground
		Vector3 position = entity->GetPosition();
		if (position.y < (float)m_groundElevation)
		{
			position.y = (float)m_groundElevation;
			entity->SetPosition(position);

			Vector3 velocity = entity->GetPhysicsComponent()->GetVelocity();
			velocity.y = 0.f;
			entity->GetPhysicsComponent()->SetVelocity(velocity);
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
			CheckAndCorrectEntityCollision(dynamicEntity, staticEntity);
		}
	}

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
			position.y = (float)m_groundElevation;
			currParticle->SetPosition(position);
			currParticle->GetPhysicsComponent()->SetVelocity(Vector3::ZERO);
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Checks for all dynamic vs. dynamic collisions in the scene, and corrects them
//
void World::CheckDynamicEntityCollisions()
{
	CheckForGroundCollisions();

	for (unsigned int layer = 0; layer < MAX_COLLISION_LAYERS; ++layer)
	{
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
					collisionDetected = collisionDetected || CheckAndCorrectEntityCollision(firstEntity, secondEntity);
				}
			}

			if (!collisionDetected)
			{
				break;
			}
		}
	}

	// Check against particles
	bool collisionDetected = false;

	for (int iteration = 0; iteration < (int)DYNAMIC_COLLISION_MAX_ITERATION_COUNT; ++iteration)
	{
		collisionDetected = false;

		for (int entityIndex = 0; entityIndex < (int)m_entities.size(); ++entityIndex)
		{
			Entity* entity = m_entities[entityIndex];

			// Don't bother with an entity marked for delete
			if (entity->GetPhysicsType() != PHYSICS_TYPE_DYNAMIC || entity->IsMarkedForDelete()) { continue; }

			for (int particleIndex = 0; particleIndex < (int)m_particles.size(); ++particleIndex)
			{
				Particle* particle = m_particles[particleIndex];

				if (particle->IsMarkedForDelete()) { continue; }

				// Do detection and fix here
				collisionDetected = collisionDetected || CheckAndCorrectEntityCollision(entity, particle);
			}
		}

		if (!collisionDetected)
		{
			break;
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Updates the navigation cost map by flagging spaces occupied by static entities as non-traversable
//
void World::UpdateCostMap()
{
	m_costsMap->Clear(1.f);

	int numEntities = (int) m_entities.size();

	for (int entityIndex = 0; entityIndex < numEntities; ++entityIndex)
	{
		Entity* currEntity = m_entities[entityIndex];

		if (!currEntity->IsMarkedForDelete() && currEntity->GetPhysicsType() == PHYSICS_TYPE_STATIC)
		{
			Vector3 position = currEntity->GetPosition();
			IntVector3 dimensions = currEntity->GetTextureForRender()->GetDimensions();
			IntVector3 halfDimensions = dimensions / 2;

			// Coordinate the object occupies (object bottom center)
			IntVector3 coordinatePosition = IntVector3(position.x, position.y, position.z);

			IntVector3 bottomLeft = coordinatePosition;
			bottomLeft.x -= halfDimensions.x;
			bottomLeft.z -= halfDimensions.z;

			for (int xOffset = 0; xOffset < dimensions.x; ++xOffset)
			{
				for (int zOffset = 0; zOffset < dimensions.z; ++zOffset)
				{
					IntVector2 coord = (bottomLeft.xz() + IntVector2(xOffset, zOffset)) / NAV_DIMENSION_FACTOR;

					if (m_costsMap->AreCoordsValid(coord))
					{
						m_costsMap->SetHeat(coord, NAV_STATIC_COST);
					}
				}
			}
		}
	}

	// Cellular automata pass
	int neighborhoodSize = 1;
	bool currCoordsDone = false;
	for (unsigned int index = 0; index < m_costsMap->GetCellCount(); ++index)
	{
		// No need to update cells that are occupied by static entities
		if (m_costsMap->GetHeat(index) >= NAV_STATIC_COST) { continue; }

		currCoordsDone = false;
		IntVector2 currCoords = m_costsMap->GetCoordsForIndex(index);

		for (int y = -neighborhoodSize; y <= neighborhoodSize; ++y)
		{
			for (int x = -neighborhoodSize; x <= neighborhoodSize; ++x)
			{
				IntVector2 neighborCoords = currCoords + IntVector2(x, y);

				if (m_costsMap->AreCoordsValid(neighborCoords) && m_costsMap->GetHeat(neighborCoords) >= NAV_STATIC_COST)
				{
					m_costsMap->AddHeat(currCoords, 5.f);
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

	int numEntities = (int)m_entities.size();

	for (int entityIndex = 0; entityIndex < numEntities; ++entityIndex)
	{
		if (m_entities[entityIndex]->GetPhysicsType() == PHYSICS_TYPE_STATIC && !m_entities[entityIndex]->IsMarkedForDelete())
		{
			if (m_drawCollision)
			{
				m_voxelGrid->DebugDrawEntityCollision(m_entities[entityIndex]);
			}
			else
			{
				m_voxelGrid->DrawEntity(m_entities[entityIndex]);
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

	int numEntities = (int)m_entities.size();

	for (int entityIndex = 0; entityIndex < numEntities; ++entityIndex)
	{
		if (m_entities[entityIndex]->GetPhysicsType() == PHYSICS_TYPE_DYNAMIC && !m_entities[entityIndex]->IsMarkedForDelete())
		{
			if (m_drawCollision)
			{
				m_voxelGrid->DebugDrawEntityCollision(m_entities[entityIndex]);
			}
			else
			{
				m_voxelGrid->DrawEntity(m_entities[entityIndex]);
			}
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Draws the particles to the voxel grid
//
void World::DrawParticlesToGrid()
{
	int numParticles = (int)m_particles.size();

	for (int i = 0; i < numParticles; ++i)
	{
		if (!m_particles[i]->IsMarkedForDelete())
		{
			m_voxelGrid->DrawEntity(m_particles[i]);
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Resolves the navigation map
//
void World::UpdatePlayerHeatmap()
{
	m_playerHeatmap->Clear(NAV_STATIC_COST);

	Player** players = Game::GetPlayers();

	// Seed in the player positions
	for (int i = 0; i < MAX_PLAYERS; ++i)
	{
		if (Game::IsPlayerAlive(i))
		{
			IntVector3 position = players[i]->GetCoordinatePosition();
			m_playerHeatmap->Seed(0.f, (position.xz() / NAV_DIMENSION_FACTOR));
		}
	}

	m_playerHeatmap->SolveMapUpToDistance(NAV_STATIC_COST + 1.f, m_costsMap);
}


//-----------------------------------------------------------------------------------------------
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


//-----------------------------------------------------------------------------------------------
// Checks if the two given entities are colliding, and pushes them out if so
//
bool CheckAndCorrectEntityCollision(Entity* first, Entity* second)
{
	// First check for team exceptions
	if (!IsThereTeamException(first, second))
	{
		return false;
	}

	CollisionDefinition_t firstDef = first->GetCollisionDefinition();
	CollisionDefinition_t secondDef = second->GetCollisionDefinition();

	BoundOverlapResult_t result = PerformBroadphaseCheck(first, second);

	if (!result.overlapOccurred)
	{
		return false;
	}

	// Now Do Narrow Phase
	bool narrowPhaseCollisionOccurred = PerformNarrowPhaseCheck(first, second, result);

	// Call collision callbacks
	if (narrowPhaseCollisionOccurred)
	{
		ApplyCollisionCorrection(first, second, result);

		first->OnCollision(second);
		second->OnCollision(first);
	}

	return narrowPhaseCollisionOccurred;
}


//-----------------------------------------------------------------------------------------------
// Checks if the AABB3 bounds of the two entities overlap eachother, and returns the result
//
BoundOverlapResult_t PerformBroadphaseCheck(Entity* first, Entity* second)
{
	Vector3 firstPosition = Vector3(first->GetCoordinatePosition());
	Vector3 secondPosition = Vector3(second->GetCoordinatePosition());

	IntVector3 firstDimensions = first->GetDimensions();
	IntVector3 secondDimensions = second->GetDimensions();

	Vector3 firstTopRight = firstPosition + Vector3(firstDimensions);
	Vector3 secondTopRight = secondPosition + Vector3(secondDimensions);

	AABB3 a = AABB3(firstPosition, firstTopRight);
	AABB3 b = AABB3(secondPosition, secondTopRight);

	// Check if a is completely to the left of b
	if (!DoAABB3sOverlap(a, b))
	{
		return BoundOverlapResult_t(false);
	}

	// 	if (a.maxs.x <= b.mins.x)
	// 	{
	// 		return BoundOverlapResult_t();
	// 		// Check if a is completely to the right of b
	// 	}
	// 	else if (a.mins.x >= b.maxs.x)
	// 	{
	// 		return false;
	// 		// Check if a is completely above b
	// 	}
	// 	else if (a.mins.y >= b.maxs.y)
	// 	{
	// 		return false;
	// 		// Check if a is completely below b
	// 	}
	// 	else if (a.maxs.y <= b.mins.y)
	// 	{
	// 		return false;
	// 	}
	// 	else if (a.maxs.z <= b.mins.z)
	// 	{
	// 		return false;
	// 	}
	// 	else if (a.mins.z >= b.maxs.z)
	// 	{
	// 		return false;
	// 	}

	BoundOverlapResult_t r = BoundOverlapResult_t(true);
	r.firstBounds = a;
	r.secondBounds = b;

	// Set up the relative position results for the first entity
	if (a.maxs.x < b.maxs.x)
	{
		if (a.mins.x > b.mins.x)
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
		if (a.mins.x > b.mins.x)
		{
			r.xCase = FIRST_ON_MAX;
		}
		else
		{
			r.xCase = FIRST_ENCAPSULATE;
		}
	}

	if (a.maxs.y < b.maxs.y)
	{
		if (a.mins.y > b.mins.y)
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
		if (a.mins.y > b.mins.y)
		{
			r.yCase = FIRST_ON_MAX;
		}
		else
		{
			r.yCase = FIRST_ENCAPSULATE;
		}
	}

	if (a.maxs.z < b.maxs.z)
	{
		if (a.mins.z > b.mins.z)
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
		if (a.mins.z > b.mins.z)
		{
			r.zCase = FIRST_ON_MAX;
		}
		else
		{
			r.zCase = FIRST_ENCAPSULATE;
		}
	}

	// Calculate the X Overlap in voxels
	r.xOverlapf = MinFloat(a.maxs.x - b.mins.x, b.maxs.x - a.mins.x);
	r.xOverlap = Ceiling(r.xOverlapf);
	r.xOverlap = ClampInt(r.xOverlap, 0, MinInt(firstDimensions.x, secondDimensions.x));

	ASSERT_OR_DIE(r.xOverlap <= a.GetDimensions().x && r.xOverlap <= b.GetDimensions().x, "Error: xOverlap too large");

	// Calculate the Y Overlap in voxels
	r.yOverlapf = MinFloat(a.maxs.y - b.mins.y, b.maxs.y - a.mins.y);
	r.yOverlap = Ceiling(r.yOverlapf);
	r.yOverlap = ClampInt(r.yOverlap, 0, MinInt(firstDimensions.y, secondDimensions.y));

	ASSERT_OR_DIE(r.yOverlap <= a.GetDimensions().y && r.yOverlap <= b.GetDimensions().y, "Error: yOverlap too large");


	// Calculate the Z Overlap in voxels
	r.zOverlapf = MinFloat(a.maxs.z - b.mins.z, b.maxs.z - a.mins.z);
	r.zOverlap = Ceiling(r.xOverlapf);
	r.zOverlap = ClampInt(r.zOverlap, 0, MinInt(firstDimensions.z, secondDimensions.z));

	ASSERT_OR_DIE(r.zOverlap <= a.GetDimensions().z && r.zOverlap <= b.GetDimensions().z, "Error: zOverlap too large");

	return r;
}



bool PerformNarrowPhaseCheck(Entity* first, Entity* second, const BoundOverlapResult_t& r)
{
	IntVector3 firstDimensions = first->GetDimensions();
	IntVector3 secondDimensions = second->GetDimensions();

	int firstYOffset = 0;
	int firstZOffset = 0;
	int secondYOffset = 0;
	int secondZOffset = 0;

	switch (r.zCase)
	{
	case FIRST_ON_MIN:
		firstZOffset = (int)firstDimensions.z - r.zOverlap;

		break;
	case FIRST_ON_MAX:
		secondZOffset = (int)secondDimensions.z - r.zOverlap;

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
		firstYOffset = (int)firstDimensions.y - r.yOverlap;

		break;
	case FIRST_ON_MAX:
		secondYOffset = (int)secondDimensions.y - r.yOverlap;

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


	const VoxelTexture* firstTexture = first->GetTextureForRender();
	const VoxelTexture* secondTexture = second->GetTextureForRender();

	for (int yIndex = 0; yIndex < r.yOverlap; ++yIndex)
	{
		for (int zIndex = 0; zIndex < r.zOverlap; ++zIndex)
		{
			uint32_t firstFlags = firstTexture->GetCollisionByteForRow(yIndex + firstYOffset, zIndex + firstZOffset);
			uint32_t secondFlags = secondTexture->GetCollisionByteForRow(yIndex + secondYOffset, zIndex + secondZOffset);

			switch (r.xCase)
			{
			case FIRST_ON_MIN:
				firstFlags = firstFlags << (firstDimensions.x - r.xOverlap);
				break;
			case FIRST_ON_MAX:
				secondFlags = secondFlags << (secondDimensions.x - r.xOverlap);
				break;
			case FIRST_INSIDE:
			{
				int bitsOnLeft = Ceiling(r.firstBounds.mins.x - r.secondBounds.mins.x);

				ASSERT_OR_DIE(bitsOnLeft >= 0, "Error: BitsOnLeft was negative");

				uint32_t mask = 0;
				int index = bitsOnLeft;
				for (int i = 0; i < r.xOverlap; ++i)
				{
					mask |= (TEXTURE_LEFTMOST_COLLISION_BIT >> index);
					++index;
				}

				secondFlags = secondFlags & mask;
				secondFlags = secondFlags << bitsOnLeft;
			}


					break;
			case FIRST_ENCAPSULATE:
			{
				int bitsOnLeft = Ceiling(r.secondBounds.mins.x - r.firstBounds.mins.x);

				ASSERT_OR_DIE(bitsOnLeft >= 0, "Error: BitsOnLeft was negative");

				uint32_t mask = 0;
				int index = bitsOnLeft;
				for (int i = 0; i < r.xOverlap; ++i)
				{
					mask |= (TEXTURE_LEFTMOST_COLLISION_BIT >> index);
					++index;
				}

				firstFlags = firstFlags & mask;
				firstFlags = firstFlags << bitsOnLeft;
			}

				break;
			}

			if ((firstFlags & secondFlags) != 0)
			{
				return true;
			}
		}
	}

	return false;
}



void ApplyCollisionCorrection(Entity* first, Entity* second, const BoundOverlapResult_t& r)
{
	Vector3 firstPosition = first->GetPosition();
	Vector3 secondPosition = second->GetPosition();

	Vector3 diff = (firstPosition - secondPosition);
	Vector3 absDiff = Vector3(AbsoluteValue(diff.x), AbsoluteValue(diff.y), AbsoluteValue(diff.z));

	Vector3 finalCorrection;
	if (r.xOverlapf < r.zOverlapf)
	{
		if (r.xOverlapf <= r.yOverlapf)
		{
			float sign = (diff.x < 0.f ? -1.f : 1.f);
			finalCorrection = Vector3(sign * r.xOverlapf, 0.f, 0.f);
		}
		else
		{
			float sign = (diff.y < 0.f ? -1.f : 1.f);
			finalCorrection = Vector3(0.f, sign * r.yOverlapf, 0.f);
		}
	}
	else
	{
		if (r.zOverlapf <= r.yOverlapf)
		{
			float sign = (diff.z < 0.f ? -1.f : 1.f);
			finalCorrection = Vector3(0.f, 0.f, sign * r.zOverlapf);
		}
		else
		{
			float sign = (diff.y < 0.f ? -1.f : 1.f);
			finalCorrection = Vector3(0.f, sign * r.yOverlapf, 0.f);
		}
	}


	float firstScalar, secondScalar;
	CalculateMassScalars(first, second, firstScalar, secondScalar);

	first->AddCollisionCorrection(firstScalar * finalCorrection);
	second->AddCollisionCorrection(-secondScalar * finalCorrection);
}


//-----------------------------------------------------------------------------------------------
// Returns the two scalars to use during a collision correction, based on the entities' masses
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
	Vector3 entityPosition = entity->GetPosition();

	unsigned int voxelCount = texture->GetVoxelCount();
	for (unsigned int i = 0; i < voxelCount; ++i)
	{
		Rgba color = texture->GetColorAtIndex(i);

		if (color.a != 0 && CheckRandomChance(0.25f))
		{
			Vector3 voxelPosition = entity->GetPositionForLocalIndex(i);

			Vector3 velocity = (voxelPosition - entityPosition).GetNormalized();
			velocity *= 50.f;

			Particle* particle = new Particle(color, 1.0f, voxelPosition, velocity);
			particle->OnSpawn();

			m_particles.push_back(particle);
		}
	}
}
