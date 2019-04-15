/************************************************************************/
/* File: EntitySpawnEvent_Meteor.cpp
/* Author: Andrew Chase
/* Date: February 12th 2019
/* Description: Implementation of the meteor spawn event
/************************************************************************/
#include "Game/Entity/Entity.hpp"
#include "Game/Framework/Game.hpp"
#include "Game/Entity/AIEntity.hpp"
#include "Game/Framework/World.hpp"
#include "Game/Framework/CampaignManager.hpp"
#include "Game/Entity/EntitySpawnEvent_Meteor.hpp"
#include "Game/Entity/Components/PhysicsComponent.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Utility/StringUtils.hpp"
#include "Engine/Core/Utility/ErrorWarningAssert.hpp"


//-----------------------------------------------------------------------------------------------
// Constructor
//
EntitySpawnEvent_Meteor::EntitySpawnEvent_Meteor(const XMLElement& element)
	: EntitySpawnEvent(element)
{
	std::string meteorDefName = ParseXmlAttribute(element, "meteor_entity", "PlayerUninitialized");
	m_definitionOfMeteorEntity = EntityDefinition::GetDefinition(meteorDefName);
	GUARANTEE_OR_DIE(m_definitionOfMeteorEntity != nullptr, Stringf("Meteor entity \"%s\" for spawn event doesn't exist", meteorDefName.c_str()).c_str());
}

#include "Engine/Core/DeveloperConsole/DevConsole.hpp"
//-----------------------------------------------------------------------------------------------
// Updates by moving the meteor down, then spawning entities when it hits
//
void EntitySpawnEvent_Meteor::Update()
{
	if (!m_meteorFinished)
	{
		if (m_meteorEntity == nullptr && IsReadyForNextSpawn())
		{
			// Allow the full sky to be a start position for the meteor
			m_meteorStartPosition = Vector3(GetRandomFloatInRange(0.f, 256.f), 64.f, GetRandomFloatInRange(0.f, 256.f));
			Vector2 targetXZ = m_areaToSpawnIn.GetRandomPointInside();

			m_meteorTargetPosition = Vector3(targetXZ.x, 0.f, targetXZ.y);

			m_fallTimer.SetInterval(m_meteorFallDuration);

			m_meteorEntity = new Entity(m_definitionOfMeteorEntity);
			m_meteorEntity->SetPosition(m_meteorStartPosition);
			m_meteorEntity->SetShouldUpdate(false);
			m_meteorEntity->SetShouldCheckForGroundCollisions(false);

			Game::GetWorld()->AddEntity(m_meteorEntity);
		}

		if (m_meteorEntity != nullptr)
		{
			Vector3 newPosition = Interpolate(m_meteorStartPosition, m_meteorTargetPosition, m_fallTimer.GetElapsedTimeNormalized());

			m_meteorEntity->SetPosition(newPosition);

			bool touchingGround = (float)(Game::GetWorld()->GetMapHeightForEntity(m_meteorEntity)) >= newPosition.y;

			if (touchingGround)
			{
				Game::GetWorld()->ParticalizeEntity(m_meteorEntity);

				IntVector3 coordPos = IntVector3(RoundToNearestInt(newPosition.x), RoundToNearestInt(newPosition.y), RoundToNearestInt(newPosition.z));
				Game::GetWorld()->ApplyExplosion(coordPos, ENTITY_TEAM_ENEMY, 5, 1.5f * (float)m_meteorEntity->GetOrientedDimensions().x, 5.f, nullptr);

				// Spawn the entities
				for (int i = 0; i < m_totalToSpawn; ++i)
				{
					AIEntity* entity = SpawnEntity(newPosition + Vector3(0.f, 20.f, 0.f), GetRandomFloatInRange(0.f, 360.f));

					Vector3 velocity = 75.f * Vector3(GetRandomFloatInRange(-1.5f, 1.5f), 1.f, GetRandomFloatInRange(-1.5f, 1.5f));
					entity->GetPhysicsComponent()->SetVelocity(velocity);
				}

				m_meteorEntity->OnDeath();
				m_meteorEntity = nullptr; // World will delete it for us
				m_meteorFinished = true;
			}
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Not used - this event spawns inside of its update
//
int EntitySpawnEvent_Meteor::RunSpawn(int maxAmountAllowedToSpawn)
{
	UNUSED(maxAmountAllowedToSpawn);
	return 0;
}


//-----------------------------------------------------------------------------------------------
// Returns a clone of this event
//
EntitySpawnEvent* EntitySpawnEvent_Meteor::Clone(CampaignManager* manager) const
{
	EntitySpawnEvent_Meteor* meteorEvent = new EntitySpawnEvent_Meteor(*this);
	meteorEvent->m_manager = manager;

	// Ensure we set up the initial spawn counts to correspond to the difficulty
	meteorEvent->RescaleToNewDifficulty(1.0f, manager->GetCurrentDifficultyScale());

	return meteorEvent;
}

