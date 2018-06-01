#pragma once
#include "Game/Behavior.hpp"
#include "Engine/Math/Vector2.hpp"

class ProjectileDefinition;

class Behavior_RangedAttack : public Behavior
{
public:
	//-----Public Methods-----

	Behavior_RangedAttack(const XMLElement& behaviorElement);

	virtual void Update(float deltaTime) override;
	virtual float CalcUtility() override;
	virtual Behavior* Clone() const override;


private:
	//-----Private Methods-----


	bool HasShotLinedUp() const;

	void FindTarget();
	void ShootWithCooldown();
	bool AttemptToMoveToShootPosition();
	void MoveTowardsTarget();
	void MoveToLastTargetPosition(float deltaTime);

private:

	Vector2 m_lastTargetPosition = Vector2::ZERO;
	const ProjectileDefinition* m_projectileToShoot;
	bool m_hasTargetPosition = false;

	float m_timeLeftToPursue;
	float m_attackCooldown;

	static constexpr float MAX_PURSUE_TIME = 10.0f;
	static constexpr float SHOTS_PER_SECOND = 1.0f;
};