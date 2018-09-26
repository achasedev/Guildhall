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
#include "Game/Entity/TestBox.hpp"

#include "Engine/Math/AABB2.hpp"
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

	TestBox* box = new TestBox();
	box->SetPosition(Vector3(50.f, 0.f, 50.f));
	m_dynamicEntities.push_back(box);

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

			if (currStatic->IsMarkedForDelete()) { continue; }

			// Do detection and fix here
			CheckAndCorrectEntityCollision(currDynamic, currStatic);
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
				collisionDetected = collisionDetected || CheckAndCorrectEntityCollision(firstEntity, secondEntity);
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

bool World::CheckAndCorrectEntityCollision(Entity* first, Entity* second)
{
	CollisionDefinition_t firstDef = first->GetCollisionDefinition();
	CollisionDefinition_t secondDef = second->GetCollisionDefinition();

	if (firstDef.m_type == COLLISION_TYPE_DISC)
	{
		if (secondDef.m_type == COLLISION_TYPE_DISC)
		{
			// Disc vs. Disc
			return CheckAndCorrect_DiscDisc(first, second);
		}
		else if (secondDef.m_type == COLLISION_TYPE_BOX)
		{
			// Disc vs. Box
			return CheckAndCorrect_BoxDisc(first, second);
		}
	}
	else if (firstDef.m_type == COLLISION_TYPE_BOX)
	{
		if (secondDef.m_type == COLLISION_TYPE_DISC)
		{
			// Box vs. Disc
			return CheckAndCorrect_BoxDisc(first, second);
		}
		else if (secondDef.m_type == COLLISION_TYPE_BOX)
		{
			// Box vs. Box
			return CheckAndCorrect_BoxBox(first, second);
		}
	}
}

float GetFirstMassScalar(Entity* first, Entity* second)
{
	float firstMass = first->GetMass();
	float secondMass = second->GetMass();

	if (firstMass > 100.f * secondMass)
	{
		return 0.f;
	}
	else if (secondMass > 100.f * firstMass)
	{
		return 1.0f;
	}

	return (firstMass / (firstMass + secondMass));
}


bool World::CheckAndCorrect_DiscDisc(Entity* first, Entity* second)
{
	CollisionDefinition_t firstDef = first->GetCollisionDefinition();
	CollisionDefinition_t secondDef = second->GetCollisionDefinition();

	float firstRadius = firstDef.m_width;
	float secondRadius = secondDef.m_width;

	Vector3 firstPosition = first->GetPosition();
	Vector3 secondPosition = second->GetPosition();

	float radiiSquared = (firstRadius + secondRadius) * (firstRadius + secondRadius);
	float distanceSquared = (firstPosition - secondPosition).GetLengthSquared();

	if (radiiSquared > distanceSquared)
	{
		// Split the difference in the overlap, based on mass
		float overlap = (firstRadius + secondRadius) - (firstPosition - secondPosition).GetLength();
		float firstScalar = GetFirstMassScalar(first, second);
		float secondScalar = 1.0f - firstScalar;

		Vector3 totalCorrection = overlap * (firstPosition - secondPosition).GetNormalized();
		first->AddCollisionCorrection(firstScalar * totalCorrection);
		second->AddCollisionCorrection(-secondScalar * totalCorrection);

		return true;
	}

	return false;
}

bool World::CheckAndCorrect_BoxDisc(Entity* first, Entity* second)
{
	bool isFirstBox = first->GetCollisionDefinition().m_type == COLLISION_TYPE_BOX;
	Entity* boxEntity;
	Entity* discEntity;

	if (isFirstBox)
	{
		boxEntity = first;
		discEntity = second;
	}
	else
	{
		boxEntity = second;
		discEntity = first;
	}

	CollisionDefinition_t boxDef = boxEntity->GetCollisionDefinition();
	CollisionDefinition_t discDef = discEntity->GetCollisionDefinition();

	Vector3 discPosition = discEntity->GetPosition();
	Vector3 boxPosition = boxEntity->GetPosition();

	float discRadius = discDef.m_width;
	Vector2 boxDimensions = Vector2(boxDef.m_width, boxDef.m_height);
	AABB2 boxBounds = AABB2(boxPosition.xz() - 0.5f * boxDimensions, boxPosition.xz() + 0.5f * boxDimensions);

	// Figure out which region the disc is in

	Vector2 edgePoint;
	if (discPosition.x < boxBounds.mins.x)	// Disc "to the left" of box
	{
		if (discPosition.z < boxBounds.mins.y) // Disc below the box
		{
			edgePoint = boxBounds.mins;
		}
		else if (discPosition.z > boxBounds.maxs.y) // Disc above the box
		{
			edgePoint = Vector2(boxBounds.mins.x, boxBounds.maxs.y);
		}
		else // Disc directly to the left
		{
			edgePoint = Vector2(boxBounds.mins.x, discPosition.z);
		}
	}
	else if (discPosition.x > boxBounds.maxs.x) // Disc "to the right" of box
	{
		if (discPosition.z < boxBounds.mins.y) // Disc below the box
		{
			edgePoint = Vector2(boxBounds.maxs.x, boxBounds.mins.y);
		}
		else if (discPosition.z > boxBounds.maxs.y) // Disc above the box
		{
			edgePoint = boxBounds.maxs;
		}
		else // Disc directly to the right
		{
			edgePoint = Vector2(boxBounds.maxs.x, discPosition.z);
		}
	}
	else // Disc within the box's x span
	{
		if (discPosition.z < boxBounds.mins.y) // Disc below the box
		{
			edgePoint = Vector2(discPosition.x, boxBounds.mins.y);
		}
		else if (discPosition.z > boxBounds.maxs.y) // Disc above the box
		{
			edgePoint = Vector2(discPosition.x, boxBounds.maxs.y);
		}
		else // Disc inside the box, special case requires additional logic
		{
			// Get closest from left right
			// Get closest from top bottom
			// Take the overall closest, and push that direction

			float topDist = boxBounds.maxs.y - discPosition.z;
			float bottomDist = discPosition.z - boxBounds.mins.y;
			float leftDist = discPosition.x - boxBounds.mins.x;
			float rightDist = boxBounds.maxs.x - discPosition.x;

			if (leftDist < rightDist)
			{
				// left is min
				if (topDist < bottomDist)
				{
					// left and top
					if (leftDist < topDist)
					{
						// left
						edgePoint = Vector2(boxBounds.mins.x, discPosition.z);
					}
					else
					{
						// top
						edgePoint = Vector2(discPosition.x, boxBounds.maxs.y);
					}
				}
				else
				{
					// left and bottom
					if (leftDist < bottomDist)
					{
						// left
						edgePoint = Vector2(boxBounds.mins.x, discPosition.z);
					}
					else
					{
						// bottom
						edgePoint = Vector2(discPosition.x, boxBounds.mins.y);
					}
				}
			}
			else
			{
				// right is min
				if (topDist < bottomDist)
				{
					// right and top
					if (rightDist < topDist)
					{
						// right
						edgePoint = Vector2(boxBounds.maxs.x, discPosition.z);
					}
					else
					{
						// top
						edgePoint = Vector2(discPosition.x, boxBounds.maxs.y);
					}
				}
				else
				{
					// right and bottom
					if (rightDist < bottomDist)
					{
						// right
						edgePoint = Vector2(boxBounds.maxs.x, discPosition.z);
					}
					else
					{
						// bottom
						edgePoint = Vector2(discPosition.x, boxBounds.mins.y);
					}
				}
			}

			// Move to the edge point, then out
			Vector2 direction = (discPosition.xz() - edgePoint);
			float totalCorrection = direction.NormalizeAndGetLength() + discRadius;
			direction *= totalCorrection;

			// Correct, since we definitely have collision
			float overlap = discRadius - (discPosition.xz() - edgePoint).GetLength();
			float firstScalar = GetFirstMassScalar(first, second);
			float secondScalar = 1.0f - firstScalar;

			Vector3 finalCorrection = Vector3(direction.x, 0.f, direction.y);
			first->AddCollisionCorrection(firstScalar * finalCorrection);
			second->AddCollisionCorrection(-secondScalar * finalCorrection);
			return true;
		}
	}

	// Got the edge point, now check for collision and correct
	float distSquared = (discPosition.xz() - edgePoint).GetLengthSquared();
	float radiusSquared = discRadius * discRadius;

	if (distSquared < radiusSquared)
	{
		// Collision Occurred, correct
		float overlap = discRadius - (discPosition.xz() - edgePoint).GetLength();
		float firstScalar = GetFirstMassScalar(first, second);
		float secondScalar = 1.0f - firstScalar;

		Vector3 totalCorrection = overlap * (discPosition - boxPosition).GetNormalized();
		first->AddCollisionCorrection(firstScalar * totalCorrection);
		second->AddCollisionCorrection(-secondScalar * totalCorrection);
		return true;
	}

	return false;
}

#include "Engine/Core/DeveloperConsole/DevConsole.hpp"
bool World::CheckAndCorrect_BoxBox(Entity* first, Entity* second)
{
	CollisionDefinition_t firstDef = first->GetCollisionDefinition();
	CollisionDefinition_t secondDef = second->GetCollisionDefinition();

	Vector3 firstPosition = first->GetPosition();
	Vector3 secondPosition = second->GetPosition();

	Vector2 firstDimensions = Vector2(firstDef.m_width, firstDef.m_length);
	AABB2 firstBounds = AABB2(firstPosition.xz() - 0.5f * firstDimensions, firstPosition.xz() + 0.5f * firstDimensions);

	Vector2 secondDimensions = Vector2(secondDef.m_width, secondDef.m_length);
	AABB2 secondBounds = AABB2(secondPosition.xz() - 0.5f * secondDimensions, secondPosition.xz() + 0.5f * secondDimensions);

	if (DoAABBsOverlap(firstBounds, secondBounds))
	{
		Vector2 diff = (firstPosition.xz() - secondPosition.xz());
		Vector2 absDiff = Vector2(AbsoluteValue(diff.x), AbsoluteValue(diff.y));
		
		float sumOfX = 0.5f * (firstDimensions.x + secondDimensions.x);
		float sumOfY = 0.5f * (firstDimensions.y + secondDimensions.y);

		float xOverlap = (sumOfX - absDiff.x);
		float yOverlap = (sumOfY - absDiff.y);

		Vector3 finalCorrection;
		if (xOverlap < yOverlap)
		{
			float sign = (diff.x < 0.f ? -1.f : 1.f);
			finalCorrection = Vector3(sign * xOverlap, 0.f, 0.f);
		}
		else
		{
			float sign = (diff.y < 0.f ? -1.f : 1.f);
			finalCorrection = Vector3(0.f, 0.f, sign * yOverlap);
		}

		float firstScalar = GetFirstMassScalar(first, second);
		float secondScalar = 1.0f - firstScalar;

		first->AddCollisionCorrection(firstScalar * finalCorrection);
		second->AddCollisionCorrection(-secondScalar * finalCorrection);
		return true;
	}

	return false;
}
