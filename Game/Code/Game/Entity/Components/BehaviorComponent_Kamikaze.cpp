/************************************************************************/
/* File: BehaviorComponent_Kamikaze.cpp
/* Author: Andrew Chase
/* Date: February 23rd 2019
/* Description: Implementation of the kamikaze behavior
/************************************************************************/
#include "Game/Entity/Player.hpp"
#include "Game/Framework/Game.hpp"
#include "Game/Framework/World.hpp"
#include "Game/Entity/AIEntity.hpp"
#include "Game/Entity/Components/PhysicsComponent.hpp"
#include "Game/Entity/Components/BehaviorComponent_Kamikaze.hpp"
#include "Engine/Math/MathUtils.hpp"

//-----------------------------------------------------------------------------------------------
// Update - moves to the targeted player and blows up when close enough
//
void BehaviorComponent_Kamikaze::Update()
{
	BehaviorComponent::Update();

	switch (m_state)
	{
	case STATE_PURSUE:
		UpdatePursue();
		break;
	case STATE_TICKING:
		UpdateTicking();
		break;
	case STATE_EXPLODE:
		UpdateExplode();
		break;
	default:
		break;
	}	
}


//-----------------------------------------------------------------------------------------------
// Clones this behavior prototype
//
BehaviorComponent* BehaviorComponent_Kamikaze::Clone() const
{
	return new BehaviorComponent_Kamikaze(*this);
}


//-----------------------------------------------------------------------------------------------
// Determines which player to attack
//
void BehaviorComponent_Kamikaze::OnSpawn()
{
	BehaviorComponent::OnSpawn();
	m_targetedPlayer = m_closestPlayer;
}


//-----------------------------------------------------------------------------------------------
// If a player touches this entity just blow up
//
void BehaviorComponent_Kamikaze::OnEntityCollision(Entity* other)
{
	if (other->IsPlayer())
	{
		m_state = STATE_EXPLODE;
	}
}


//-----------------------------------------------------------------------------------------------
// Update for chasing after a target player
//
void BehaviorComponent_Kamikaze::UpdatePursue()
{
	// Need to swap targets if our target dies
	if (m_targetedPlayer == nullptr || m_targetedPlayer->IsRespawning())
	{
		m_targetedPlayer = m_closestPlayer;

		// Do nothing if there is no player
		if (m_targetedPlayer == nullptr)
		{
			m_owningEntity->Decelerate();
			return;
		}
	}


	Vector3 targetPlayerPosition = m_targetedPlayer->GetPosition();
	Vector3 currPosition = m_owningEntity->GetPosition();

	Vector2 directionToPlayer = (targetPlayerPosition - currPosition).xz();
	float distanceToTargetPlayer = directionToPlayer.NormalizeAndGetLength();

	Vector2 directionToAvoidObstacle = GetDirectionToAvoidClosestStaticObstacle(directionToPlayer);

	// Sum directions together
	Vector2 finalDirection = (0.5f * directionToAvoidObstacle + 0.5f * directionToPlayer).GetNormalized();

	if (distanceToTargetPlayer <= DISTANCE_TO_EXPLODE)
	{
		m_tickTimer.SetInterval(TICK_DURATION);
		m_state = STATE_TICKING;
	}
	else
	{
		m_owningEntity->Move(finalDirection);	
		m_owningEntity->SetOrientation(directionToPlayer.GetOrientationDegrees());
	}
}


//-----------------------------------------------------------------------------------------------
// Update for when the entity is ticking down to explode
//
void BehaviorComponent_Kamikaze::UpdateTicking()
{
	// Stop them on the player
	m_owningEntity->GetPhysicsComponent()->StopAllMovement();

	float t = m_tickTimer.GetElapsedTimeNormalized();

	if (t >= 0.25f && m_normalizedTimeLastFrame < 0.25f || t >= 0.5f && m_normalizedTimeLastFrame < 0.5f || t >= 0.75f && m_normalizedTimeLastFrame < 0.75f)
	{
		m_owningEntity->SetColorOverride(Rgba::RED);

		// Play the tick sound
		AudioSystem* audio = AudioSystem::GetInstance();
		audio->PlaySound(m_owningEntity->GetEntityDefinition()->m_customSound);
	}

	m_normalizedTimeLastFrame = t;

	if (m_tickTimer.HasIntervalElapsed())
	{
		m_state = STATE_EXPLODE;
	}
}


//-----------------------------------------------------------------------------------------------
// Blows up this entity and marks it for delete
//
void BehaviorComponent_Kamikaze::UpdateExplode()
{
	IntVector3 entityCenterCoord = m_owningEntity->GetBottomCenterCoordinatePosition();
	Game::GetWorld()->ApplyExplosion(entityCenterCoord, ENTITY_TEAM_ENEMY, m_explosionDamage, m_explosionRadius, m_explosionKnockback, m_owningEntity);

	m_owningEntity->OnDeath();
}
