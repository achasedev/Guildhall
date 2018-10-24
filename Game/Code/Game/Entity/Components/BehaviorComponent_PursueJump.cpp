/************************************************************************/
/* File: BehaviorComponent_PursueJump.cpp
/* Author: Andrew Chase
/* Date: October 15th 2018
/* Description: Implementation of the PursueJump behavior
/************************************************************************/
#include "Game/Entity/Player.hpp"
#include "Game/Framework/Game.hpp"
#include "Game/Framework/World.hpp"
#include "Game/Entity/Components/BehaviorComponent_PursueJump.hpp"
#include "Engine/Core/Utility/ErrorWarningAssert.hpp"


//-----------------------------------------------------------------------------------------------
// Constructor
//
BehaviorComponent_PursueJump::BehaviorComponent_PursueJump()
{
}


//-----------------------------------------------------------------------------------------------
// Initializes the component by placing the jump sensor just outside the entity's collision boundary
//
void BehaviorComponent_PursueJump::Initialize(MovingEntity* owningEntity)
{
	BehaviorComponent::Initialize(owningEntity);
	m_jumpSensorDistance = m_owningEntity->GetCollisionDefinition().m_xExtent + SENSOR_OFFSET_DISTANCE;
}


//-----------------------------------------------------------------------------------------------
// Update
//
void BehaviorComponent_PursueJump::Update()
{
	Vector2 jumpSensorOffset = Vector2::MakeDirectionAtDegrees(m_owningEntity->GetOrientation()) * m_jumpSensorDistance;
	Vector3 jumpSensorPosition = m_owningEntity->GetEntityPosition() + Vector3(jumpSensorOffset.x, 0.f, jumpSensorOffset.y);

	// Move the entity
	BehaviorComponent::Update();

	Player** players = Game::GetPlayers();

	Vector3 closestPlayerPosition;
	float minDistance = 9999.f;
	bool playerFound = false;

	Vector3 currentPosition = m_owningEntity->GetEntityPosition();

	for (int i = 0; i < MAX_PLAYERS; ++i)
	{
		if (Game::IsPlayerAlive(i))
		{
			Vector3 playerPosition = players[i]->GetEntityPosition();
			float currDistance = (playerPosition - currentPosition).GetLengthSquared();

			if (!playerFound || currDistance < minDistance)
			{
				minDistance = currDistance;
				closestPlayerPosition = playerPosition;
				playerFound = true;
			}
		}
	}

	// Shouldn't happen, but to avoid unidentifiable behavior
	if (!playerFound)
	{
		return;
	}

	Vector3 directionToMove = (closestPlayerPosition - currentPosition).GetNormalized();
	m_owningEntity->Move(directionToMove.xz());

	// Check for jumping
	bool shouldJump = (Game::GetWorld()->IsPositionInStatic(jumpSensorPosition));

	if (shouldJump)
	{
		m_owningEntity->Jump();
	}
}


//-----------------------------------------------------------------------------------------------
// Clones this behavior and returns it
//
BehaviorComponent* BehaviorComponent_PursueJump::Clone() const
{
	ASSERT_OR_DIE(m_owningEntity == nullptr, "Error: Behavior clone had non-null base members on prototype");
	return new BehaviorComponent_PursueJump();
}
