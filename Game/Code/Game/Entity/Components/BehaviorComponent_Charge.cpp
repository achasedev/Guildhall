/************************************************************************/
/* File: BehaviorComponent_Charge.cpp
/* Author: Andrew Chase
/* Date: November 26th 2018
/* Description: Implementation of the charge behavior
/************************************************************************/
#include "Game/Entity/Player.hpp"
#include "Game/Framework/Game.hpp"
#include "Game/Entity/Components/PhysicsComponent.hpp"
#include "Game/Entity/Components/BehaviorComponent_Charge.hpp"
#include "Engine/Math/MathUtils.hpp"


//-----------------------------------------------------------------------------------------------
// Constructor
//
BehaviorComponent_Charge::BehaviorComponent_Charge()
	: m_stateTimer(Game::GetGameClock())
{
}


//-----------------------------------------------------------------------------------------------
// Update
//
void BehaviorComponent_Charge::Update()
{
	Player* playerInSight = GetClosestPlayerInSight();

	switch (m_state)
	{
	case STATE_SEARCH:

		if (playerInSight == nullptr)
		{
			MoveToClosestPlayer();
		}
		else
		{
			m_owningEntity->GetPhysicsComponent()->StopAllMovement();
			m_chargeDirection = (playerInSight->GetPosition() - m_owningEntity->GetPosition()).xz().GetNormalized();
			m_owningEntity->SetOrientation(m_chargeDirection.GetOrientationDegrees());
			m_owningEntity->Jump();
			m_state = STATE_JUMP;
		}
		break;
	case STATE_JUMP:
		// Check for landing
		if (m_owningEntity->IsGrounded())
		{
			m_stateTimer.SetInterval(m_chargeDuration);
			m_state = STATE_CHARGE;
			m_owningEntity->Move(m_chargeDirection, m_chargeSpeed);
		}
		break;
	case STATE_CHARGE:
		ContinueCharge();
		break;
	case STATE_KNOCKBACK:
		// Check for landing
		if (m_owningEntity->IsGrounded())
		{
			m_stateTimer.SetInterval(m_restDuration);
			m_state = STATE_REST;
		}
		break;
	case STATE_REST:
		if (m_stateTimer.HasIntervalElapsed())
		{
			m_state = STATE_SEARCH;
		}
		break;
	default:
		break;
	}
}


//-----------------------------------------------------------------------------------------------
// Clones the behavior
//
BehaviorComponent* BehaviorComponent_Charge::Clone() const
{
	BehaviorComponent_Charge* charge = new BehaviorComponent_Charge();

	charge->m_chargeSpeed = m_chargeSpeed;
	charge->m_restDuration = m_restDuration;
	charge->m_chargeDuration = m_chargeDuration;
	charge->m_damageOnCharge = m_damageOnCharge;
	charge->m_damageOnTouch = m_damageOnTouch;
	charge->m_knockbackMagnitude = m_knockbackMagnitude;

	return charge;
}


//-----------------------------------------------------------------------------------------------
// Stops a charge if the entity runs into something
//
void BehaviorComponent_Charge::OnEntityCollision(Entity* other)
{
	if (m_state == STATE_CHARGE)
	{
		Vector2 forward = Vector2::MakeDirectionAtDegrees(m_owningEntity->GetOrientation());
		Vector2 directionToOther = (other->GetBottomCenterPosition() - m_owningEntity->GetBottomCenterPosition()).xz().GetNormalized();

		if (DotProduct(forward, directionToOther) > 0.8f)
		{
			m_owningEntity->GetPhysicsComponent()->StopAllMovement();
			m_state = STATE_KNOCKBACK;

			Vector2 selfKnockbackDirection = -1.f * m_chargeDirection;
			m_owningEntity->GetPhysicsComponent()->AddImpulse(m_knockbackMagnitude * Vector3(selfKnockbackDirection.x, 1.f, selfKnockbackDirection.y));

			// Also apply damage and knockback, if on other team
			if (other->GetTeam() != m_owningEntity->GetTeam())
			{
				Vector3 otherKnockback = 1.5f * m_knockbackMagnitude * Vector3(m_chargeDirection.x, 0.f, m_chargeDirection.y);
				other->TakeDamage(m_damageOnCharge, otherKnockback);
			}
		}
	}
	else
	{
		// No knockback, and the touch damage
		if (other->GetTeam() != m_owningEntity->GetTeam())
		{
			other->TakeDamage(m_damageOnTouch);
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Continues a charge in the moving direction for a set distance
//
void BehaviorComponent_Charge::ContinueCharge()
{
	m_owningEntity->GetPhysicsComponent()->GetVelocity();

	// Charged long enough
	if (m_stateTimer.HasIntervalElapsed())
	{
		m_owningEntity->GetPhysicsComponent()->StopAllMovement();
		m_state = STATE_REST;
		m_stateTimer.SetInterval(m_restDuration);
	}
	else // Continue charge
	{
		m_owningEntity->Move(m_chargeDirection, m_chargeSpeed);
	}
}
