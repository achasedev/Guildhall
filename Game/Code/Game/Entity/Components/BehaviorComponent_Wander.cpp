/************************************************************************/
/* File: BehaviorComponent_Wander.cpp
/* Author: Andrew Chase
/* Date: March 29th 2019
/* Description: Implementation of the Wander BehaviorComponent
/************************************************************************/
#include "Game/Framework/Game.hpp"
#include "Game/Entity/AIEntity.hpp"
#include "Game/Entity/Components/BehaviorComponent_Wander.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Utility/ErrorWarningAssert.hpp"


//-----------------------------------------------------------------------------------------------
// Update
//
void BehaviorComponent_Wander::Update()
{
	switch (m_state)
	{
	case WANDER_STATE_MOVING:
		UpdateMove();
		break;
	case WANDER_STATE_WAIT:
		UpdateWait();
		break;
	default:
		break;
	}
}


//-----------------------------------------------------------------------------------------------
// Cloning for duplication from a definition
//
BehaviorComponent* BehaviorComponent_Wander::Clone() const
{
	ASSERT_OR_DIE(m_owningEntity == nullptr, "Error: Behavior clone had non-null base members on prototype");
	return new BehaviorComponent_Wander(*this);
}


//-----------------------------------------------------------------------------------------------
// Rotates towards the target and moves in the forward direction
//
void BehaviorComponent_Wander::UpdateMove()
{
	Vector2 directionToTarget = m_targetPosition - m_owningEntity->GetPosition().xz();

	// Termination check
	if (directionToTarget.GetLengthSquared() < 10.f || m_stateTimer.HasIntervalElapsed())
	{
		m_state = WANDER_STATE_WAIT;
		float waitTime = GetRandomFloatInRange(WANDER_BASE_WAIT_TIME - WANDER_WAIT_DEVIANCE, WANDER_BASE_WAIT_TIME + WANDER_WAIT_DEVIANCE);
		m_stateTimer.SetInterval(waitTime);
	}

	float deltaTime = Game::GetDeltaTime();
	float maxDegreesToMove = WANDER_ROTATION_SPEED * deltaTime;
	float currentOrientation = m_owningEntity->GetOrientation();
	float goalOrientation = directionToTarget.GetOrientationDegrees();

	float newOrientation = TurnToward(currentOrientation, goalOrientation, maxDegreesToMove);
	m_owningEntity->SetOrientation(newOrientation);

	Vector2 forwardDirection = m_owningEntity->GetForwardVector().xz();
	m_owningEntity->Move(forwardDirection);

}


//-----------------------------------------------------------------------------------------------
// Returns a new wander target position
//
Vector2 GetNewWanderTarget()
{
	float x = GetRandomFloatInRange(0.f, 255.f);
	float z = GetRandomFloatInRange(0.f, 255.f);

	return Vector2(x, z);
}


//-----------------------------------------------------------------------------------------------
// Stands still and finds a new target when done waiting
//
void BehaviorComponent_Wander::UpdateWait()
{
	m_owningEntity->Decelerate();

	if (m_stateTimer.HasIntervalElapsed())
	{
		m_targetPosition = GetNewWanderTarget();

		m_state = WANDER_STATE_MOVING;
		m_stateTimer.SetInterval(WANDER_MAX_MOVE_TIME);
	}
}
