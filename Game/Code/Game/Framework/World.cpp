/************************************************************************/
/* File: VoxelGrid.cpp
/* Author: Andrew Chase
/* Date: September 15th, 2018
/* Description: Implementation of the world class
/************************************************************************/
#include "Game/Entity/Player.hpp"
#include "Game/Framework/World.hpp"
#include "Game/Framework/VoxelGrid.hpp"
#include "Game/Entity/StaticEntity.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Time/ProfileLogScoped.hpp"

#define DYNAMIC_COLLISION_MAX_ITERATION_COUNT 5

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

}


//-----------------------------------------------------------------------------------------------
// Initializes the grid and any other setup
//
void World::Inititalize()
{
	m_players[0] = new Player(0);
	m_players[0]->SetPosition(Vector3(60.f, 0.f, 60.f));

	m_voxelGrid = new VoxelGrid();
	m_voxelGrid->Initialize(IntVector3(256, 64, 256));

	m_dynamicEntities.push_back(m_players[0]);

// 	DynamicEntity* test = new DynamicEntity();
// 	test->SetPosition(Vector3(50.f, 0.f, 50.f));
// 	m_dynamicEntities.push_back(test);
}


//-----------------------------------------------------------------------------------------------
// Update
//
void World::Update()
{
	PROFILE_LOG_SCOPE_FUNCTION();

	// "Thinking" and other general updating (animation)
	ProcessPlayerInput();
	UpdateStaticEntities();
	UpdateDynamicEntities();

	// Moving the entities (Forward Euler)
	ApplyPhysicsStep();

	// Collision
	CheckStaticEntityCollisions();
	CheckDynamicEntityCollisions();

	// Clean Up
	DeleteMarkedEntities();
}


//-----------------------------------------------------------------------------------------------
// Render
//
void World::Render()
{
	PROFILE_LOG_SCOPE_FUNCTION();

	// Clear grid
	m_voxelGrid->Clear();

	// Color in static geometry
	DrawStaticEntitiesToGrid();

	// Color in each entity (shouldn't overlap, this is after physics step)
	DrawDynamicEntitiesToGrid();

	// Rebuild the mesh and draw it to screen
	m_voxelGrid->BuildMeshAndDraw();
}

void World::AddDynamicEntity(DynamicEntity* entity)
{
	m_dynamicEntities.push_back(entity);
	entity->OnSpawn();
}

void World::AddStaticEntity(StaticEntity* entity)
{
	m_staticEntities.push_back(entity);
	entity->OnSpawn();
}

void World::ProcessPlayerInput()
{
	for (int i = 0; i < MAX_PLAYERS; ++i)
	{
		if (m_players[i] != nullptr)
		{
			m_players[i]->ProcessInput();
		}
	}
}

void World::UpdateStaticEntities()
{
	int numStatics = m_staticEntities.size();

	for (int i = 0; i < numStatics; ++i)
	{
		StaticEntity* currStatic = m_staticEntities[i];

		if (!currStatic->IsMarkedForDelete())
		{
			m_staticEntities[i]->Update();
		}
	}
}

void World::UpdateDynamicEntities()
{
	int numDynamics = m_dynamicEntities.size();

	for (int i = 0; i < numDynamics; ++i)
	{
		DynamicEntity* currDynamic = m_dynamicEntities[i];

		if (!currDynamic->IsMarkedForDelete())
		{
			m_dynamicEntities[i]->Update();
		}
	}
}

void World::ApplyPhysicsStep()
{
	int numDynamics = m_dynamicEntities.size();

	for (int i = 0; i < numDynamics; ++i)
	{
		DynamicEntity* currDynamic = m_dynamicEntities[i];

		if (!currDynamic->IsMarkedForDelete())
		{
			m_dynamicEntities[i]->ApplyPhysicsStep();
		}
	}
}

void World::CheckStaticEntityCollisions()
{
	for (int dynamicIndex = 0; dynamicIndex < (int)m_dynamicEntities.size(); ++dynamicIndex)
	{
		DynamicEntity* currDynamic = m_dynamicEntities[dynamicIndex];
		
		// Don't bother with an entity marked for delete
		if (currDynamic->IsMarkedForDelete()) { continue; }

		for (int staticIndex = 0; staticIndex < (int)m_staticEntities.size(); ++staticIndex)
		{
			StaticEntity* currStatic = m_staticEntities[staticIndex];

			if (currStatic->IsMarkedForDelete()) { continue;  }

			// Do detection and fix here
			float overlap = GetEntityOverlap(currDynamic, currStatic);

			if (overlap > 0)
			{
				// Move the dynamic entity back
				Vector3 correction = overlap * (currDynamic->GetPosition() - currStatic->GetPosition()).GetNormalized();
				currDynamic->AddCollisionCorrection(correction);
			}
		}
	}
}

void World::CheckDynamicEntityCollisions()
{
	bool collisionDetected;

	for (int iteration = 0; iteration < (int) DYNAMIC_COLLISION_MAX_ITERATION_COUNT; ++iteration)
	{
		collisionDetected = false;

		for (int firstIndex = 0; firstIndex < (int)m_dynamicEntities.size() - 1; ++firstIndex)
		{
			DynamicEntity* firstEntity = m_dynamicEntities[firstIndex];

			// Don't bother with an entity marked for delete
			if (firstEntity->IsMarkedForDelete()) { continue; }

			for (int secondIndex = firstIndex + 1; secondIndex < (int)m_dynamicEntities.size(); ++secondIndex)
			{
				DynamicEntity* secondEntity = m_dynamicEntities[secondIndex];

				if (secondEntity->IsMarkedForDelete()) { continue; }

				// Do detection and fix here
				float overlap = GetEntityOverlap(firstEntity, secondEntity);

				if (overlap > 0)
				{
					// Split the difference in the overlap, based on mass
					float firstScalar = firstEntity->GetMass() / (firstEntity->GetMass() + secondEntity->GetMass());
					float secondScalar = 1.0f - firstScalar;

					Vector3 totalCorrection = overlap * (firstEntity->GetPosition() - secondEntity->GetPosition()).GetNormalized();
					firstEntity->AddCollisionCorrection(firstScalar * totalCorrection);
					secondEntity->AddCollisionCorrection(-secondScalar * totalCorrection);

					collisionDetected = true;
				}
			}
		}

		if (!collisionDetected)
		{
			break;
		}
	}
}

void World::DeleteMarkedEntities()
{
	// Static first
	for (int i = (int)m_staticEntities.size() - 1; i >= 0; --i)
	{
		StaticEntity* currStatic = m_staticEntities[i];

		if (currStatic->IsMarkedForDelete())
		{
			m_staticEntities[i]->OnDeath();
			delete m_staticEntities[i];

			if (i < (int)m_staticEntities.size() - 1)
			{
				m_staticEntities[i] = m_staticEntities.back();
			}
			
			m_staticEntities.pop_back();
		}
	}

	for (int i = (int)m_dynamicEntities.size() - 1; i >= 0; --i)
	{
		DynamicEntity* currDynamic = m_dynamicEntities[i];

		if (currDynamic->IsMarkedForDelete())
		{
			m_dynamicEntities[i]->OnDeath();
			delete m_dynamicEntities[i];

			if (i < (int)m_dynamicEntities.size() - 1)
			{
				m_dynamicEntities[i] = m_dynamicEntities.back();
			}

			m_dynamicEntities.pop_back();
		}
	}
}


void World::DrawStaticEntitiesToGrid()
{
	int numStatics = (int)m_staticEntities.size();

	for (int staticIndex = 0; staticIndex < numStatics; ++staticIndex)
	{
		m_voxelGrid->DrawEntity(m_staticEntities[staticIndex]);
	}
}

void World::DrawDynamicEntitiesToGrid()
{
	int numDynamics = m_dynamicEntities.size();

	for (int i = 0; i < numDynamics; ++i)
	{
		m_voxelGrid->DrawEntity(m_dynamicEntities[i]);
	}
}

float World::GetEntityOverlap(Entity* first, Entity* second)
{
	float firstRadius = first->GetCollisionRadius();
	float secondRadius = second->GetCollisionRadius();

	Vector3 firstPosition = first->GetPosition();
	Vector3 secondPosition = second->GetPosition();

	float radiiSquared = (firstRadius + secondRadius) * (firstRadius + secondRadius);
	float distanceSquared = (firstPosition - secondPosition).GetLengthSquared();

	if (radiiSquared > distanceSquared)
	{
		float correctionSquared = radiiSquared - distanceSquared;
		return Sqrt(correctionSquared);
	}
	
	return 0;
}
