/************************************************************************/
/* File: BehaviorComponent_SwarmAndAvoid.cpp
/* Author: Andrew Chase
/* Date: February 23rd 2018
/* Description: Implementation of the SwarmAndAvoid behavior
/************************************************************************/
#include "Game/Framework/Game.hpp"
#include "Game/Entity/Player.hpp"
#include "Game/Framework/Game.hpp"
#include "Game/Framework/World.hpp"
#include "Game/Entity/Components/BehaviorComponent_SwarmAndAvoid.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Time/Clock.hpp"
#include "Engine/Core/Utility/ErrorWarningAssert.hpp"


//-----------------------------------------------------------------------------------------------
// Update
//
void BehaviorComponent_SwarmAndAvoid::Update()
{
	BehaviorComponent::Update();

	if (m_closestPlayer != nullptr)
	{
		Vector3 direction3D = (m_closestPlayer->GetCenterPosition() - m_owningEntity->GetCenterPosition());
		Vector2 directionToPlayer = direction3D.xz().GetNormalized();
		Vector2 entityForward = m_owningEntity->GetForwardVector().xz();
		Vector2 entityPosition = m_owningEntity->GetPosition().xz();

		std::vector<Entity*> entities = Game::GetWorld()->GetEntitiesThatOverlapSphere(m_owningEntity->GetCenterPosition(), 10.f);
		Vector2 directionToClosestStaticObstacle = Vector2::ZERO;
		Vector2 sumOfVectorsAwayFromNearbySwarmMembers = Vector2::ZERO;
		float distanceToClosestStatic = 10000.f;

		// Check all nearby entities
		for (int entityIndex = 0; entityIndex < (int)entities.size(); ++entityIndex)
		{
			Entity* entity = entities[entityIndex];

			if (entity == m_owningEntity) // Don't compare to ourselves
			{
				continue;
			}
			else if (entity->GetTeam() == ENTITY_TEAM_ENEMY) // Other enemy, space ourselves out from it if too close
			{
				Vector2 directionToEnemy = entity->GetCenterPosition().xz() - m_owningEntity->GetCenterPosition().xz();
				float currDistance = directionToEnemy.NormalizeAndGetLength();

				if (currDistance < m_elbowRoom)
				{
					sumOfVectorsAwayFromNearbySwarmMembers -= directionToEnemy;
				}
			}
			else if (entity->GetPhysicsType() == PHYSICS_TYPE_STATIC) // Walk around a static
			{
				Vector2 directionToObstacle = entity->GetCenterPosition().xz() - m_owningEntity->GetCenterPosition().xz();
				float currDistance = directionToObstacle.NormalizeAndGetLength();

				bool withinCone = DotProduct(directionToObstacle, entityForward) > CosDegrees(45.f);

				if (withinCone && currDistance < distanceToClosestStatic)
				{
					distanceToClosestStatic = currDistance;
					directionToClosestStaticObstacle = directionToObstacle;
				}
			}
		}

		Vector2 directionToAvoidObstacle = Vector2::ZERO;
		if (directionToClosestStaticObstacle != Vector2::ZERO)
		{
			float sign = (GetAngularDisplacement(directionToPlayer.GetOrientationDegrees(), directionToClosestStaticObstacle.GetOrientationDegrees()) < 0.f ? 1.0f : -1.0f);
			directionToAvoidObstacle = sign * Vector2(-directionToClosestStaticObstacle.y, directionToClosestStaticObstacle.x);
		}

		if (sumOfVectorsAwayFromNearbySwarmMembers != Vector2::ZERO)
		{
			sumOfVectorsAwayFromNearbySwarmMembers.NormalizeAndGetLength();
		}

		// Sum directions together
		Vector2 finalDirection = (0.2f * sumOfVectorsAwayFromNearbySwarmMembers + 0.5f * directionToAvoidObstacle + 0.3f * directionToPlayer).GetNormalized();

		m_owningEntity->Move(finalDirection);

		// Make sure the entity is still looking at the player
		m_owningEntity->SetOrientation(directionToPlayer.GetOrientationDegrees());
	}
}


//-----------------------------------------------------------------------------------------------
// Makes a copy of this behavior and returns it
//
BehaviorComponent* BehaviorComponent_SwarmAndAvoid::Clone() const
{
	ASSERT_OR_DIE(m_owningEntity == nullptr, "Error: Behavior clone had non-null base members on prototype");
	return new BehaviorComponent_SwarmAndAvoid();
}
