/************************************************************************/
/* File: BehaviorComponent_Smash.cpp
/* Author: Andrew Chase
/* Date: February 23rd 2019
/* Description: Implemenation of the Smash behavior
/************************************************************************/
#include "Game/Entity/Player.hpp"
#include "Game/Framework/Game.hpp"
#include "Game/Framework/Game.hpp"
#include "Game/Framework/World.hpp"
#include "Game/Entity/AIEntity.hpp"
#include "Game/Entity/Components/BehaviorComponent_Smash.hpp"
#include "Engine/Core/Utility/ErrorWarningAssert.hpp"

//-----------------------------------------------------------------------------------------------
// Constructor
//
BehaviorComponent_Smash::BehaviorComponent_Smash()
{
	m_waitTimer.Reset();
}


//-----------------------------------------------------------------------------------------------
// Used to shut off physics when the enemy spawns
//
void BehaviorComponent_Smash::Initialize(AIEntity* owningEntity)
{
	BehaviorComponent::Initialize(owningEntity);
	m_owningEntity->SetPhysicsEnabled(false);
}


//-----------------------------------------------------------------------------------------------
// Update
//
void BehaviorComponent_Smash::Update()
{
	BehaviorComponent::Update();

	switch (m_state)
	{
	case STATE_WAITING_ON_GROUND:
		UpdateWaitingOnGround();
		break;
	case STATE_MOVING_TO_HOVER_TARGET:
		UpdateMovingToHoverTarget();
		break;
	case STATE_HOVERING:
		UpdateHovering();
		break;
	case STATE_SMASHING:
		UpdateSmashing();
		break;
	default:
		break;
	}
}


//-----------------------------------------------------------------------------------------------
// Returns a clone of this behavior
//
BehaviorComponent* BehaviorComponent_Smash::Clone() const
{
	ASSERT_OR_DIE(m_owningEntity == nullptr, "Error: Behavior clone had non-null base members on prototype");
	return new BehaviorComponent_Smash(*this);
}


//-----------------------------------------------------------------------------------------------
// Override for when the entity smash collides into a player
//
void BehaviorComponent_Smash::OnEntityCollision(Entity* other)
{
	if (other->GetTeam() == ENTITY_TEAM_PLAYER)
	{
		if (m_state == STATE_SMASHING)
		{
			Vector3 knockback = (other->GetCenterPosition() - m_owningEntity->GetCenterPosition()).GetNormalized();
			other->TakeDamage(m_damageOnSmash, knockback * m_smashKnockBackMagnitude);
		}
		else
		{
			BehaviorComponent::OnEntityCollision(other);
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Update for when the entity is sitting on the ground waiting for the next move
//
void BehaviorComponent_Smash::UpdateWaitingOnGround()
{
	if (m_waitTimer.HasIntervalElapsed() && m_closestPlayer != nullptr)
	{
		// Find a target
		Vector3 playerPosition = m_closestPlayer->GetCenterPosition();
		m_hoverTarget = playerPosition + Vector3(0.f, HOVER_HEIGHT_ABOVE_PLAYER, 0.f);
		m_hoverDirection = (m_hoverTarget - m_owningEntity->GetCenterPosition()).GetNormalized();

		m_state = STATE_MOVING_TO_HOVER_TARGET;
	}
}


//-----------------------------------------------------------------------------------------------
// Moves the entity to the hover position above the targeted player
//
void BehaviorComponent_Smash::UpdateMovingToHoverTarget()
{
	float deltaTime = Game::GetDeltaTime();
	Vector3 translation = m_hoverDirection * HOVER_TRANSLATION_SPEED * deltaTime;
	m_owningEntity->AddPositionOffset(translation);

	float thresholdSquared = TRANSLATION_THRESHOLD_ACCURACY * TRANSLATION_THRESHOLD_ACCURACY;
	float distanceToTargetSquared = (m_hoverTarget - m_owningEntity->GetCenterPosition()).GetLengthSquared();

	if (distanceToTargetSquared < thresholdSquared)
	{
		m_owningEntity->SetCenterPosition(m_hoverTarget);

		m_waitTimer.SetInterval(HOVER_WAIT_TIME);
		m_state = STATE_HOVERING;
	}
}


//-----------------------------------------------------------------------------------------------
// Update for when the entity is above a player
//
void BehaviorComponent_Smash::UpdateHovering()
{
	if (m_waitTimer.HasIntervalElapsed())
	{
		m_waitTimer.Reset();
		m_state = STATE_SMASHING;
	}
}


//-----------------------------------------------------------------------------------------------
// Update for when the entity is falling down on a player
//
void BehaviorComponent_Smash::UpdateSmashing()
{
	if (!m_owningEntity->IsGrounded())
	{
		float deltaTime = Game::GetDeltaTime();
		m_owningEntity->AddPositionOffset(Vector3(0.f, -HOVER_TRANSLATION_SPEED * deltaTime, 0.f));
	}
	else
	{
		m_waitTimer.SetInterval(GROUND_WAIT_TIME);
		m_state = STATE_WAITING_ON_GROUND;

		Game::GetWorld()->DestroyPartOfMap(m_owningEntity->GetBottomCenterCoordinatePosition(), (float)m_owningEntity->GetOrientedDimensions().x, 50.f, 2);
	}
}
