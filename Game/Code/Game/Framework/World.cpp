/************************************************************************/
/* File: VoxelGrid.cpp
/* Author: Andrew Chase
/* Date: September 15th, 2018
/* Description: Implementation of the world class
/************************************************************************/
#include "Game/Entity/Player.hpp"
#include "Game/Entity/TestBox.hpp"
#include "Game/Framework/Game.hpp"
#include "Game/Entity/Particle.hpp"
#include "Game/Framework/World.hpp"
#include "Game/Framework/VoxelGrid.hpp"
#include "Game/Entity/StaticEntity.hpp"
#include "Game/Framework/GameCamera.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Assets/AssetDB.hpp"
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
void World::Inititalize(const char* filename)
{
	m_terrain = AssetDB::CreateOrGet3DVoxelTextureInstance(filename);

	m_dimensions = IntVector3(256, 64, 256);
	//m_dimensions = m_terrain->GetDimensions();

	m_voxelGrid = new VoxelGrid();
	m_voxelGrid->Initialize(m_dimensions);

	m_groundElevation = 4;
}


//-----------------------------------------------------------------------------------------------
// Update
//
void World::Update()
{
	PROFILE_LOG_SCOPE_FUNCTION();

	// "Thinking" and other general updating (animation)
	UpdateStaticEntities();
	UpdateDynamicEntities();
	UpdateParticles();

	// Moving the entities (Forward Euler)
	ApplyPhysicsStep();

	// Collision
	CheckDynamicEntityCollisions();
	CheckStaticEntityCollisions();

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

	// Color in the terrain
	DrawTerrainToGrid();

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
// Adds the given dynamic entity to the world
//
void World::AddDynamicEntity(DynamicEntity* entity)
{
	m_dynamicEntities.push_back(entity);
	entity->OnSpawn();
}


//-----------------------------------------------------------------------------------------------
// Adds the given static entity to the world
//
void World::AddStaticEntity(StaticEntity* entity)
{
	m_staticEntities.push_back(entity);
	entity->OnSpawn();
}


//-----------------------------------------------------------------------------------------------
// Blows up the given entity
//
void World::ParticalizeEntity()
{
	DynamicEntity* entity = m_dynamicEntities[1];

	m_dynamicEntities.erase(m_dynamicEntities.begin() + 1);

	Texture3D* texture = entity->GetTextureForOrientation();
	Vector3 entityPosition = entity->GetEntityPosition();

	unsigned int voxelCount = texture->GetVoxelCount();
	for (unsigned int i = 0; i < voxelCount; ++i)
	{
		//Rgba color = texture->GetColorAtIndex(i);
		Rgba color = Rgba::GetRandomColor();

		if (color.a != 0)
		{
			Vector3 voxelPosition = entity->GetPositionForLocalIndex(i);

			Vector3 velocity = (voxelPosition - entityPosition).GetNormalized();
			velocity *= 50.f;

			Particle* particle = new Particle(color, 300.0f, voxelPosition, velocity);
			particle->OnSpawn();

			m_particles.push_back(particle);
		}
	}

	delete entity;
}


//-----------------------------------------------------------------------------------------------
// Updates the static entities in the scene
//
void World::UpdateStaticEntities()
{
	int numStatics = (int) m_staticEntities.size();

	for (int i = 0; i < numStatics; ++i)
	{
		StaticEntity* currStatic = m_staticEntities[i];

		if (!currStatic->IsMarkedForDelete())
		{
			m_staticEntities[i]->Update();
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Updates the dynamic entities in the scene
//
void World::UpdateDynamicEntities()
{
	int numDynamics = (int) m_dynamicEntities.size();

	for (int i = 0; i < numDynamics; ++i)
	{
		DynamicEntity* currDynamic = m_dynamicEntities[i];

		if (!currDynamic->IsMarkedForDelete())
		{
			m_dynamicEntities[i]->Update();
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Updates all particles in the game
//
void World::UpdateParticles()
{
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
	int numDynamics = (int) m_dynamicEntities.size();

	for (int i = 0; i < numDynamics; ++i)
	{
		DynamicEntity* currDynamic = m_dynamicEntities[i];

		if (!currDynamic->IsMarkedForDelete())
		{
			m_dynamicEntities[i]->ApplyPhysicsStep();
		}
	}

	// Apply it to particles too
	int numParticles = (int)m_particles.size();

	for (int i = 0; i < numParticles; ++i)
	{
		if (!m_particles[i]->IsMarkedForDelete())
		{
			m_particles[i]->ApplyPhysicsStep();
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Checks for all dynamic vs. static collisions in the scene, and corrects them
//
void World::CheckStaticEntityCollisions()
{
	for (int particleIndex = 0; particleIndex < (int)m_dynamicEntities.size(); ++particleIndex)
	{
		DynamicEntity* currDynamic = m_dynamicEntities[particleIndex];
		
		// Don't bother with an entity marked for delete
		if (currDynamic->IsMarkedForDelete()) { continue; }

		for (int staticIndex = 0; staticIndex < (int)m_staticEntities.size(); ++staticIndex)
		{
			StaticEntity* currStatic = m_staticEntities[staticIndex];

			if (currStatic->IsMarkedForDelete()) { continue; }

			// Do detection and fix here
			CheckAndCorrectEntityCollision(currDynamic, currStatic);
		}

		// Also check for clipping into the ground
		Vector3 position = currDynamic->GetEntityPosition();
		if (position.y < (float)m_groundElevation)
		{
			position.y = (float) m_groundElevation;
			currDynamic->SetPosition(position);

			Vector3 velocity = currDynamic->GetVelocity();
			velocity.y = 0.f;
			currDynamic->SetVelocity(velocity);
		}
	}

	// Check particles for ground collisions
	for (int particleIndex = 0; particleIndex < (int)m_particles.size(); ++particleIndex)
	{
		DynamicEntity* currParticle = m_particles[particleIndex];

		// Don't bother with particles marked for delete
		if (currParticle->IsMarkedForDelete()) { continue; }

		// Also check for clipping into the ground
		Vector3 position = currParticle->GetEntityPosition();
		if (position.y < (float)m_groundElevation)
		{
			position.y = (float)m_groundElevation;
			currParticle->SetPosition(position);
			currParticle->SetVelocity(Vector3::ZERO);
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Checks for all dynamic vs. dynamic collisions in the scene, and corrects them
//
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

	// Check against particles
	for (int iteration = 0; iteration < (int)DYNAMIC_COLLISION_MAX_ITERATION_COUNT; ++iteration)
	{
		collisionDetected = false;

		for (int firstIndex = 0; firstIndex < (int)m_dynamicEntities.size(); ++firstIndex)
		{
			DynamicEntity* firstEntity = m_dynamicEntities[firstIndex];

			// Don't bother with an entity marked for delete
			if (firstEntity->IsMarkedForDelete()) { continue; }

			for (int secondIndex = 0; secondIndex < (int)m_particles.size(); ++secondIndex)
			{
				DynamicEntity* secondEntity = m_particles[secondIndex];

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


//-----------------------------------------------------------------------------------------------
// Deletes all entities marked for delete this frame, except for the players
//
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

	// Check for players before going into the dynamic list
	// Players will set themselves back to not marked, preventing deletion
	Player** players = Game::GetPlayers();

	for (int i = 0; i < MAX_PLAYERS; ++i)
	{
		if (players[i] != nullptr && players[i]->IsMarkedForDelete())
		{
			players[i]->OnDeath();
		}
	}

	// Check dynamics
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
// Draws the terrain to the grid
//
void World::DrawTerrainToGrid()
{
	PROFILE_LOG_SCOPE_FUNCTION();
	m_voxelGrid->Draw3DTexture(m_terrain, IntVector3(0, 0, 0));
}


//-----------------------------------------------------------------------------------------------
// Draws all static entities into the voxel grid
//
void World::DrawStaticEntitiesToGrid()
{
	PROFILE_LOG_SCOPE_FUNCTION();

	int numStatics = (int)m_staticEntities.size();

	for (int staticIndex = 0; staticIndex < numStatics; ++staticIndex)
	{
		m_voxelGrid->DrawEntity(m_staticEntities[staticIndex]);
	}
}


//-----------------------------------------------------------------------------------------------
// Draws all dynamic entities into the grid
//
void World::DrawDynamicEntitiesToGrid()
{
	PROFILE_LOG_SCOPE_FUNCTION();

	int numDynamics = (int) m_dynamicEntities.size();

	for (int i = 0; i < numDynamics; ++i)
	{
		m_voxelGrid->DrawEntity(m_dynamicEntities[i]);
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
// Checks if the two given entities are colliding, and pushes them out if so
//
bool World::CheckAndCorrectEntityCollision(Entity* first, Entity* second)
{
	CollisionDefinition_t firstDef = first->GetCollisionDefinition();
	CollisionDefinition_t secondDef = second->GetCollisionDefinition();

	if (firstDef.m_shape == COLLISION_SHAPE_DISC)
	{
		if (secondDef.m_shape == COLLISION_SHAPE_DISC)
		{
			// Disc vs. Disc
			return CheckAndCorrect_DiscDisc(first, second);
		}
		else if (secondDef.m_shape == COLLISION_SHAPE_BOX)
		{
			// Disc vs. Box
			return CheckAndCorrect_BoxDisc(first, second);
		}
	}
	else if (firstDef.m_shape == COLLISION_SHAPE_BOX)
	{
		if (secondDef.m_shape == COLLISION_SHAPE_DISC)
		{
			// Box vs. Disc
			return CheckAndCorrect_BoxDisc(first, second);
		}
		else if (secondDef.m_shape == COLLISION_SHAPE_BOX)
		{
			// Box vs. Box
			return CheckAndCorrect_BoxBox(first, second);
		}
	}

	// Else we don't check collision, so just return false
	return false;
}


//-----------------------------------------------------------------------------------------------
// Returns the two scalars to use during a collision correction, based on the entities' masses
//
void GetMassScalars(Entity* first, Entity* second, float& out_firstScalar, float& out_secondScalar)
{
	CollisionDefinition_t firstDef = first->GetCollisionDefinition();
	CollisionDefinition_t secondDef = second->GetCollisionDefinition();

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
// Checks if two disc-collision entities overlap, and corrects them if so
//
bool World::CheckAndCorrect_DiscDisc(Entity* first, Entity* second)
{
	CollisionDefinition_t firstDef = first->GetCollisionDefinition();
	CollisionDefinition_t secondDef = second->GetCollisionDefinition();

	float firstRadius = firstDef.m_xExtent;
	float secondRadius = secondDef.m_xExtent;

	Vector3 firstPosition = first->GetEntityPosition();
	Vector3 secondPosition = second->GetEntityPosition();

	// Height check
	bool firstAboveSecond = (secondPosition.y + secondDef.m_height) < (firstPosition.y);
	bool secondAboveFirst = (firstPosition.y + firstDef.m_height) < (secondPosition.y);

	if (firstAboveSecond || secondAboveFirst)
	{
		return false;
	}

	float radiiSquared = (firstRadius + secondRadius) * (firstRadius + secondRadius);
	float distanceSquared = (firstPosition - secondPosition).GetLengthSquared();

	if (radiiSquared > distanceSquared)
	{
		// Split the difference in the overlap, based on mass
		float overlap = (firstRadius + secondRadius) - (firstPosition - secondPosition).GetLength();
		float firstScalar, secondScalar;
		GetMassScalars(first, second, firstScalar, secondScalar);

		Vector3 totalCorrection = overlap * (firstPosition - secondPosition).GetNormalized();
		first->AddCollisionCorrection(firstScalar * totalCorrection);
		second->AddCollisionCorrection(-secondScalar * totalCorrection);

		return true;
	}

	return false;
}


//-----------------------------------------------------------------------------------------------
// Checks if a disc and a box entity are overlapping, and corrects them if so
//
bool World::CheckAndCorrect_BoxDisc(Entity* first, Entity* second)
{
	bool isFirstBox = first->GetCollisionDefinition().m_shape == COLLISION_SHAPE_BOX;
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

	Vector3 discPosition = discEntity->GetEntityPosition();
	Vector3 boxPosition = boxEntity->GetEntityPosition();

	float discRadius	= discDef.m_xExtent;
	Vector2 boxExtents	= Vector2(boxDef.m_xExtent, boxDef.m_zExtent);
	AABB2 boxBounds		= AABB2(boxPosition.xz() - boxExtents, boxPosition.xz() + boxExtents);

	// Height check
	bool discAboveBox = ((boxPosition.y + boxDef.m_height) < (discPosition.y - discDef.m_height));
	bool boxAboveDisc = (boxPosition.y > (discPosition.y + discDef.m_height));

	if (discAboveBox || boxAboveDisc)
	{
		return false;
	}

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
			float firstScalar, secondScalar;
			GetMassScalars(first, second, firstScalar, secondScalar);

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
		float firstScalar, secondScalar;
		GetMassScalars(first, second, firstScalar, secondScalar);

		Vector3 totalCorrection = overlap * (discPosition - boxPosition).GetNormalized();
		first->AddCollisionCorrection(firstScalar * totalCorrection);
		second->AddCollisionCorrection(-secondScalar * totalCorrection);
		return true;
	}

	return false;
}


//-----------------------------------------------------------------------------------------------
// Checks if 2 box-collision entities are overlapping, and corrects them if so
//
bool World::CheckAndCorrect_BoxBox(Entity* first, Entity* second)
{
	CollisionDefinition_t firstDef = first->GetCollisionDefinition();
	CollisionDefinition_t secondDef = second->GetCollisionDefinition();

	Vector3 firstPosition	= first->GetEntityPosition();
	Vector3 secondPosition	= second->GetEntityPosition();

	Vector2 firstExtents	= Vector2(firstDef.m_xExtent, firstDef.m_zExtent);
	Vector2 secondExtents	= Vector2(secondDef.m_xExtent, secondDef.m_zExtent);

	AABB2 firstBounds	= AABB2(firstPosition.xz() - firstExtents, firstPosition.xz() + firstExtents);
	AABB2 secondBounds	= AABB2(secondPosition.xz() - secondExtents, secondPosition.xz() + secondExtents);

	// Height check
	bool firstAboveSecond = (secondPosition.y + secondDef.m_height) < (firstPosition.y);
	bool secondAboveFirst = (firstPosition.y + firstDef.m_height) < (secondPosition.y);

	if (firstAboveSecond || secondAboveFirst)
	{
		return false;
	}

	if (DoAABBsOverlap(firstBounds, secondBounds))
	{
		Vector2 diff = (firstPosition.xz() - secondPosition.xz());
		Vector2 absDiff = Vector2(AbsoluteValue(diff.x), AbsoluteValue(diff.y));
		
		float sumOfX = (firstExtents.x + secondExtents.x);
		float sumOfY = (firstExtents.y + secondExtents.y);

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

		float firstScalar, secondScalar;
		GetMassScalars(first, second, firstScalar, secondScalar);

		first->AddCollisionCorrection(firstScalar * finalCorrection);
		second->AddCollisionCorrection(-secondScalar * finalCorrection);
		return true;
	}

	return false;
}
