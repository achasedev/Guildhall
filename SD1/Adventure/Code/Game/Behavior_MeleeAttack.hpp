#pragma once
#include "Game/Behavior.hpp"
#include "Engine/Math/Vector2.hpp"

class Behavior_MeleeAttack : public Behavior
{
public:
	//-----Public Methods-----

	Behavior_MeleeAttack(const XMLElement& behaviorElement);

	virtual void Update(float deltaTime) override;
	virtual float CalcUtility() override;
	virtual Behavior* Clone() const override;


private:
	//-----Private Methods-----

	void FindTarget();
	bool CloseToTarget() const;
	void AttackWithCooldown();
	void MoveTowardsTarget();
	void MoveToLastTargetPosition(float deltaTime);

private:

	Vector2 m_lastTargetPosition = Vector2::ZERO;
	bool m_hasTargetPosition = false;

	float m_timeLeftToPursue;
	float m_attackCooldown;

	static constexpr float MAX_PURSUE_TIME = 10.0f;
	static constexpr float ATTACKS_PER_SECOND = 1.0f;
	static constexpr float MELEE_RANGE = 1.0f;
};