#include "Game/Behavior_MeleeAttack.hpp"
#include "Game/Actor.hpp"
#include "Game/Map.hpp"
#include "Game/ProjectileDefinition.hpp"
#include "Game/Projectile.hpp"
#include "Engine/Math/MathUtils.hpp"

Behavior_MeleeAttack::Behavior_MeleeAttack(const XMLElement& behaviorElement)
	: Behavior(behaviorElement)
	, m_timeLeftToPursue(MAX_PURSUE_TIME)
	, m_attackCooldown(ATTACKS_PER_SECOND)
{
}

void Behavior_MeleeAttack::Update(float deltaTime)
{
	if (m_target != nullptr)
	{
		m_lastTargetPosition = m_target->GetPosition();
		m_hasTargetPosition = true;

		if (CloseToTarget())
		{
			AttackWithCooldown();
		}

		MoveTowardsTarget();
		
	}
	else if (m_hasTargetPosition)
	{
		MoveToLastTargetPosition(deltaTime);
	}

	m_attackCooldown -= deltaTime;
}

float Behavior_MeleeAttack::CalcUtility()
{
	FindTarget();

	return 5.f;
}

Behavior* Behavior_MeleeAttack::Clone() const
{
	return new Behavior_MeleeAttack(*this);
}

void Behavior_MeleeAttack::FindTarget()
{
	m_target = m_actor->GetMap()->FindBestHostileActor(m_actor);	
}

bool Behavior_MeleeAttack::CloseToTarget() const
{
	Vector2 targetPos = m_target->GetPosition();
	Vector2 currPos = m_actor->GetPosition();

	float distance = (targetPos - currPos).GetLength();

	return (distance < MELEE_RANGE);
}

void Behavior_MeleeAttack::AttackWithCooldown()
{
	Vector2 displacement = (m_target->GetPosition() - m_actor->GetPosition());
	float faceAngle = GetNearestCardinalAngle(displacement.GetOrientationDegrees());
	m_actor->SetOrientationDegrees(faceAngle);

	if (m_attackCooldown <= 0.f)
	{
		m_attackCooldown = (1.f / ATTACKS_PER_SECOND);

		// Fire the projectile
		Vector2 projectileSpawnPosition = m_actor->GetPosition() + (Vector2::MakeDirectionAtDegrees(faceAngle) * m_actor->GetInnerRadius());

		Projectile* meleeProjectile = m_actor->GetMap()->SpawnProjectile(projectileSpawnPosition, faceAngle, ProjectileDefinition::GetDefinitionByName("Melee"), "", m_actor->GetFaction());
		meleeProjectile->SetStats(m_actor->GetStats());

		// Set the animation on the actor to show shooting - based on cardinal angle
		if		(faceAngle == 0.f)		{ m_actor->StartAnimation("SpearAttackEast"); }
		else if (faceAngle == 90.f)		{ m_actor->StartAnimation("SpearAttackNorth"); }
		else if (faceAngle == 180.f)	{ m_actor->StartAnimation("SpearAttackWest"); }
		else if (faceAngle == 270.f)	{ m_actor->StartAnimation("SpearAttackSouth"); }

	}
}

void Behavior_MeleeAttack::MoveTowardsTarget()
{
	Vector2 targetPos = m_target->GetPosition();
	Vector2 currPos = m_actor->GetPosition();

	IntVector2 targetCoords = IntVector2(targetPos.x, targetPos.y);
	IntVector2 currCoords = IntVector2(currPos.x, currPos.y);

	SetActorVelocityAlongPath(targetCoords);
}


void Behavior_MeleeAttack::MoveToLastTargetPosition(float deltaTime)
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

