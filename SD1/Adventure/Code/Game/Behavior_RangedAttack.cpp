#include "Game/Behavior_RangedAttack.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Map.hpp"
#include "Game/Actor.hpp"
#include "Game/ProjectileDefinition.hpp"
#include "Game/Projectile.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/MathUtils.hpp"


Behavior_RangedAttack::Behavior_RangedAttack(const XMLElement& behaviorElement)
	: Behavior(behaviorElement)
	, m_timeLeftToPursue(MAX_PURSUE_TIME)
	, m_attackCooldown(SHOTS_PER_SECOND)
{
	// Parse ranged attack data here
	m_projectileToShoot = ProjectileDefinition::ParseXMLAttribute(behaviorElement, "projectile", nullptr);
}

void Behavior_RangedAttack::Update(float deltaTime)
{
	if (m_target != nullptr)
	{
		m_lastTargetPosition = m_target->GetPosition();
		m_hasTargetPosition = true;

		if (HasShotLinedUp())
		{
			ShootWithCooldown();
		}
		else 
		{
			bool couldMoveToShootPosition = AttemptToMoveToShootPosition();

			if (!couldMoveToShootPosition)
			{
				MoveTowardsTarget();
			}
		}		
	}
	else if (m_hasTargetPosition)
	{
		MoveToLastTargetPosition(deltaTime);
	}

	m_attackCooldown -= deltaTime;
}

float Behavior_RangedAttack::CalcUtility()
{
	// Find target for utility calculation and Update() if this behavior has top utility 
	FindTarget();

	// If you still see a target
	if (m_target != nullptr && m_actor->GetMap()->HasLineOfSight(m_target->GetPosition(), m_actor->GetPosition(), m_actor->GetViewDistance()))
	{
		return 0.8f;
	}
	// If you still have a leftover target position
	else if (m_hasTargetPosition)
	{
		return 0.6f;
	}

	// Can't attack anything so low utility
	return 0.1f;
}

Behavior* Behavior_RangedAttack::Clone() const
{
	return new Behavior_RangedAttack(*this);
}

void Behavior_RangedAttack::FindTarget()
{
	m_target = m_actor->GetMap()->FindBestHostileActor(m_actor);
}

bool Behavior_RangedAttack::HasShotLinedUp() const
{
	Vector2 displacementToTarget = m_target->GetPosition() - m_actor->GetPosition();
	float angleToTarget = displacementToTarget.GetOrientationDegrees();

	float nearestShootDegrees = GetNearestCardinalAngle(angleToTarget);

	// Only shoot if within 10 degree cone
	float difference = GetAngularDisplacement(nearestShootDegrees, angleToTarget);

	if (difference < 0.f)
	{
		difference *= -1.f;
	}
	
	if (difference < 2.f)
	{
		return true;
	}
	else
	{
		return false;
	}
}


void Behavior_RangedAttack::ShootWithCooldown()
{
	// Stop moving and turn towards target to shoot
	m_actor->SetVelocity(Vector2::ZERO);

	Vector2 displacement = (m_target->GetPosition() - m_actor->GetPosition());
	float faceAngle = GetNearestCardinalAngle(displacement.GetOrientationDegrees());
	m_actor->SetOrientationDegrees(faceAngle);

	if (m_attackCooldown <= 0.f)
	{
		m_attackCooldown = (1.f / SHOTS_PER_SECOND);

		// Fire the projectile
		Vector2 projectileSpawnPosition = m_actor->GetPosition() + (Vector2::MakeDirectionAtDegrees(faceAngle) * m_actor->GetInnerRadius());

		Projectile* arrowProjectile = m_actor->GetMap()->SpawnProjectile(projectileSpawnPosition, faceAngle, m_projectileToShoot, "", m_actor->GetFaction());
		arrowProjectile->SetStats(m_actor->GetStats());

		// Set the animation on the actor to show shooting - based on cardinal angle
		if		(faceAngle == 0.f) { m_actor->StartAnimation("RangedAttackEast"); }
		else if (faceAngle == 90.f) { m_actor->StartAnimation("RangedAttackNorth"); }
		else if (faceAngle == 180.f) { m_actor->StartAnimation("RangedAttackWest"); }
		else if (faceAngle == 270.f) { m_actor->StartAnimation("RangedAttackSouth"); }
		
	}
}

bool Behavior_RangedAttack::AttemptToMoveToShootPosition()
{
	// Find the shoot position to use
	Vector2 targetPos = m_target->GetPosition();
	Vector2 actorPos = m_actor->GetPosition();

	Vector2 verticalShootPosition = Vector2(targetPos.x, actorPos.y);
	Vector2 horizontalShootPosition = Vector2(actorPos.x, targetPos.y);


	// Choose the closer point that is in line of sight
	Map* actorMap = m_actor->GetMap();
	
	float vertSquaredDistance = -1;
	if (actorMap->HasLineOfSight(actorPos, verticalShootPosition, 9999.f))
	{
		vertSquaredDistance = (verticalShootPosition - actorPos).GetLengthSquared();
	}
	
	float horizSquaredDistance = -1;
	if (actorMap->HasLineOfSight(actorPos, horizontalShootPosition, 9999.f))
	{
		horizSquaredDistance = (horizontalShootPosition - actorPos).GetLengthSquared();
	}

	// Set the next position based on line of sight and distance
	Vector2 moveToPosition = Vector2::ZERO;
	if (vertSquaredDistance < horizSquaredDistance && vertSquaredDistance != -1.f)
	{
		moveToPosition = verticalShootPosition;
	}
	else if (horizSquaredDistance != -1.f)
	{
		moveToPosition = horizontalShootPosition;
	}
	else
	{
		return false;
	}


	// Set velocity to correspond with that move
	Vector2 directionToPosition = (moveToPosition - actorPos).GetNormalized();
	float walkSpeed = m_actor->GetWalkSpeed();

	m_actor->SetVelocity(directionToPosition * walkSpeed);
	
	// Return true indicating we found a position
	return true;
}


void Behavior_RangedAttack::MoveTowardsTarget()
{
	Vector2 targetPos = m_target->GetPosition();
	Vector2 currPos = m_actor->GetPosition();

	IntVector2 targetCoords = IntVector2(targetPos.x, targetPos.y);
	IntVector2 currCoords = IntVector2(currPos.x, currPos.y);

	SetActorVelocityAlongPath(targetCoords);
}

void Behavior_RangedAttack::MoveToLastTargetPosition(float deltaTime)
{
	Vector2 currPos = m_actor->GetPosition();

	IntVector2 targetCoords = IntVector2(m_lastTargetPosition.x, m_lastTargetPosition.y);
	IntVector2 currCoords = IntVector2(currPos.x, currPos.y);

	SetActorVelocityAlongPath(targetCoords);
	
	// Check if we should stop pursuing
	float distanceToTarget = (m_lastTargetPosition - currPos).GetLength();
	if (distanceToTarget < m_actor->GetInnerRadius() || m_timeLeftToPursue <= 0.f)	// Reached the position or the timer is up
	{
		m_hasTargetPosition = false;
		m_timeLeftToPursue = MAX_PURSUE_TIME;
		m_actor->SetVelocity(Vector2::ZERO);
	}
	else
	{
		m_timeLeftToPursue -= deltaTime;
	}
}

