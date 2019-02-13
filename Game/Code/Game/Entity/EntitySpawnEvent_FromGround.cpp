/************************************************************************/
/* File: EntitySpawnEvent_FromGround.cpp
/* Author: Andrew Chase
/* Date: February 12th 2019
/* Description: Implementation of the FromGround spawn event
/************************************************************************/
#include "Game/Framework/Game.hpp"
#include "Game/Framework/World.hpp"
#include "Game/Entity/AIEntity.hpp"
#include "Game/Entity/EntitySpawnEvent_FromGround.hpp"
#include "Game/Entity/Components/PhysicsComponent.hpp"
#include "Engine/Math/MathUtils.hpp"

//-----------------------------------------------------------------------------------------------
// Constructor from an XML element
//
EntitySpawnEvent_FromGround::EntitySpawnEvent_FromGround(const XMLElement& element)
	: EntitySpawnEvent(element)
{
}


//-----------------------------------------------------------------------------------------------
// Updates by incrementing the position of all entities, and when they're done rising
// allowing them to update and function
//
void EntitySpawnEvent_FromGround::Update()
{
	int numRisingEntities = (int)m_entitiesRisingFromGround.size();

	for (int riseIndex = numRisingEntities - 1; riseIndex >= 0; --riseIndex)
	{
		RisingEntity_t& risingEntity = m_entitiesRisingFromGround[riseIndex];

		// Check if the entity was killed during rising, and remove immediately
		if (!IsEventTrackingThisEntity(risingEntity.entity))
		{
			m_entitiesRisingFromGround.erase(m_entitiesRisingFromGround.begin() + riseIndex);
			continue;
		}

		// If we're done rising, let the entity update and have ground collision
		if (m_entitiesRisingFromGround[riseIndex].riseTimer.HasIntervalElapsed())
		{
			risingEntity.entity->SetShouldUpdate(true);
			risingEntity.entity->SetShouldCheckForGroundCollisions(true);
			risingEntity.entity->GetPhysicsComponent()->SetGravity(true);

			m_entitiesRisingFromGround.erase(m_entitiesRisingFromGround.begin() + riseIndex);
		}
		else
		{
			// Rise the entity up to the proper height
			float newYPos = Interpolate(0.f, (float) risingEntity.entity->GetOrientedDimensions().y, risingEntity.riseTimer.GetElapsedTimeNormalized()) + risingEntity.startingHeight;
			
			Vector3 currentPosition = risingEntity.entity->GetPosition();
			Vector3 newPosition = currentPosition;
			newPosition.y = newYPos;

			risingEntity.entity->SetPosition(newPosition);
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Spawns the correct number of entities and adds them to the rising entity list
//
int EntitySpawnEvent_FromGround::RunSpawn()
{
	int countToSpawn = MinInt(m_spawnRate, GetEntityCountLeftToSpawn());

	for (int spawnNumber = 0; spawnNumber < countToSpawn; ++spawnNumber)
	{
		IntVector3 dimensions = m_definitionToSpawn->GetDimensions();
		IntVector2 spawnXZ = IntVector2(m_areaToSpawnIn.GetRandomPointInside()) - (dimensions.xz() / 2);

		float orientation = GetRandomFloatInRange(0.f, 360.f);

		int height = Game::GetWorld()->GetMapHeightForBounds(IntVector3(spawnXZ.x, 0, spawnXZ.y), dimensions.xz());
		height -= dimensions.y;

		Vector3 spawnPosition = Vector3(spawnXZ.x, height, spawnXZ.y);

		AIEntity* entity = SpawnEntity(spawnPosition, orientation);

		entity->SetShouldCheckForGroundCollisions(false);
		entity->SetShouldUpdate(false);
		entity->GetPhysicsComponent()->SetGravity(false);

		RisingEntity_t risingEntity;
		risingEntity.entity = entity;
		risingEntity.riseTimer.SetInterval(m_riseTime);
		risingEntity.startingHeight = height;

		m_entitiesRisingFromGround.push_back(risingEntity);
	}

	return countToSpawn;
}


//-----------------------------------------------------------------------------------------------
// Returns a spawn event of this subclass
//
EntitySpawnEvent* EntitySpawnEvent_FromGround::Clone(CampaignManager* manager) const
{
	EntitySpawnEvent_FromGround* groundSpawnEvent = new EntitySpawnEvent_FromGround(*this);
	groundSpawnEvent->m_manager = manager;

	return groundSpawnEvent;
}
