#pragma once
#include "Game/Behavior.hpp"
#include "Engine/Math/Vector2.hpp"

class Behavior_Wander : public Behavior
{
public:
	//-----Public Methods-----

	Behavior_Wander(const XMLElement& behaviorElement);

	virtual void Update(float deltaTime) override;
	virtual float CalcUtility() override;
	virtual Behavior* Clone() const override;


private:
	//-----Private Methods-----

	void UpdateWanderTarget();
	Vector2 GetLocalWanderPosition() const;

private:

	Vector2 m_wanderPosition = Vector2::ZERO;			// The position the actor is fleeing to
	float m_wanderTimer;

	int m_maxDistancePerMove = 2;

	static constexpr float WANDER_TIME = 5.f;
};