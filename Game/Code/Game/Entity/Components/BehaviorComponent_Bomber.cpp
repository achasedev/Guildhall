/************************************************************************/
/* File: BehaviorComponent_Bomber.cpp
/* Author: Andrew Chase
/* Date: February 23rd 2019
/* Description: Implementation of the bomber behavior
/************************************************************************/
#include "Game/Framework/Game.hpp"
#include "Game/Framework/World.hpp"
#include "Game/Entity/Projectile.hpp"
#include "Game/Entity/AnimatedEntity.hpp"
#include "Game/Entity/Components/PhysicsComponent.hpp"
#include "Game/Entity/Components/BehaviorComponent_Bomber.hpp"
#include "Engine/Math/MathUtils.hpp"

//-----------------------------------------------------------------------------------------------
// Constructor - takes the projectile that this entity will drop
//
BehaviorComponent_Bomber::BehaviorComponent_Bomber(const EntityDefinition* projectileDefinition)
{
	m_projectileDefinition = projectileDefinition;
}


void BehaviorComponent_Bomber::Initialize(AnimatedEntity* owningEntity)
{
	BehaviorComponent::Initialize(owningEntity);
	owningEntity->SetPhysicsEnabled(false);
}


//-----------------------------------------------------------------------------------------------
// Update
//
void BehaviorComponent_Bomber::Update()
{
	if (!m_spawned)
	{
		FindNewTargetPosition();

		// Get the enemy a little off the ground
		Vector3 position = m_owningEntity->GetPosition();
		position.y = TARGET_HEIGHT_OFF_GROUND;
		m_owningEntity->SetPosition(position);

		m_bombTimer.SetInterval(m_bombCooldown);
		m_spawned = true;
	}

	float deltaTime = Game::GetDeltaTime();
	Vector3 translation = m_movingDirection * m_moveSpeed * deltaTime;
	m_owningEntity->AddPositionOffset(translation);

	float distanceToTargetSquared = (m_targetPosition - m_owningEntity->GetCenterPosition()).GetLengthSquared();
	float thresholdSquared = TARGET_MOVING_THRESHOLD * TARGET_MOVING_THRESHOLD;
	
	if (distanceToTargetSquared < thresholdSquared)
	{
		FindNewTargetPosition();
	}

	if (m_bombTimer.HasIntervalElapsed())
	{
		Projectile* bomb = new Projectile(m_projectileDefinition, m_owningEntity->GetTeam());
		bomb->GetPhysicsComponent()->SetGravity(true);
		bomb->SetPosition(m_owningEntity->GetBottomCenterPosition());

		Game::GetWorld()->AddEntity(bomb);

		m_bombTimer.SetInterval(m_bombCooldown);
	}
}


//-----------------------------------------------------------------------------------------------
// Returns a clone of this prototype
//
BehaviorComponent* BehaviorComponent_Bomber::Clone() const
{
	return new BehaviorComponent_Bomber(*this);
}

#include "Engine/Core/DeveloperConsole/DevConsole.hpp"
//-----------------------------------------------------------------------------------------------
// Finds a new position to move to that is away from the entity
//
void BehaviorComponent_Bomber::FindNewTargetPosition()
{
	// Ensure the next target is at least min distance away
	Vector3 target;
	bool done = false;
	while (!done)
	{
		target.x = GetRandomFloatInRange(20.f, 236.f);
		target.z = GetRandomFloatInRange(20.f, 236.f);

		target.y = (float)Game::GetWorld()->GetMapHeightForPosition(target.xz()) + TARGET_HEIGHT_OFF_GROUND;

		// Ensure the target is far enough away
		Vector3 directionToTarget = target - m_owningEntity->GetCenterPosition();
		float distanceSquared = directionToTarget.NormalizeAndGetLength();
		distanceSquared *= distanceSquared;

		if (distanceSquared >= MIN_DISTANCE_TO_NEW_TARGET_SQUARED)
		{
			m_movingDirection = directionToTarget;
			m_targetPosition = target;
			done = true;

			ConsolePrintf("Start: (%.2f, %.2f, %.2f) | Target: (%.2f, %.2f, %.2f) | Direction: (%.2f, %.2f, %.2f)",
				m_owningEntity->GetCenterPosition().x, m_owningEntity->GetCenterPosition().y, m_owningEntity->GetCenterPosition().z,
				target.x, target.y, target.z, directionToTarget.x, directionToTarget.y, directionToTarget.z);
		}
	}
}
