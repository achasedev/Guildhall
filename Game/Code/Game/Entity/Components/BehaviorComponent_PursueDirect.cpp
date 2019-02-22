/************************************************************************/
/* File: BehaviorComponent_PursueDirect.cpp
/* Author: Andrew Chase
/* Date: October 15th 2018
/* Description: Implementation of the PursueDirect behavior
/************************************************************************/
#include "Game/Entity/Player.hpp"
#include "Game/Framework/Game.hpp"
#include "Game/Framework/World.hpp"
#include "Game/Entity/Components/BehaviorComponent_PursueDirect.hpp"
#include "Engine/Core/Utility/ErrorWarningAssert.hpp"


//-----------------------------------------------------------------------------------------------
// Update
//
void BehaviorComponent_PursueDirect::Update()
{
	BehaviorComponent::Update();

	if (m_closestPlayer != nullptr)
	{
		Vector3 direction3D = (m_closestPlayer->GetCenterPosition() - m_owningEntity->GetCenterPosition()).GetNormalized();
		Vector2 direction2D = direction3D.xz();

		// Check for nearby static obstacles
		std::vector<Entity*> entities = Game::GetWorld()->GetEntitiesThatOverlapSphere(m_owningEntity->GetCenterPosition(), 10.f);
		Vector2 repulsion = Vector2::ZERO;
		float minDistance = 10000.f;

		for (int entityIndex = 0; entityIndex < (int)entities.size(); ++entityIndex)
		{
			Entity* entity = entities[entityIndex];

			if (entity == m_owningEntity || entity->GetPhysicsType() != PHYSICS_TYPE_STATIC)
			{
				continue;
			}

			Vector2 directionToObstacle = entity->GetCenterPosition().xz() - m_owningEntity->GetCenterPosition().xz();
			float currDistance = directionToObstacle.NormalizeAndGetLength();

			if (currDistance < minDistance)
			{
				minDistance = currDistance;
				repulsion = directionToObstacle;
			}
		}

		Vector2 sideStepDirection = 2.f * Vector2(-repulsion.y, repulsion.x);
		Vector2 finalDirection = (sideStepDirection + direction2D).GetNormalized();

		m_owningEntity->Move(finalDirection);
		m_owningEntity->SetOrientation(direction2D.GetOrientationDegrees());
		// Check for holes

		
	}
}


//-----------------------------------------------------------------------------------------------
// Makes a copy of this behavior and returns it
//
BehaviorComponent* BehaviorComponent_PursueDirect::Clone() const
{
	ASSERT_OR_DIE(m_owningEntity == nullptr, "Error: Behavior clone had non-null base members on prototype");
	return new BehaviorComponent_PursueDirect();
}
