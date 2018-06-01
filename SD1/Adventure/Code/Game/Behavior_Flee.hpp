#pragma once
#include "Game/Behavior.hpp"
#include "Engine/Math/Vector2.hpp"

class Behavior_Flee : public Behavior
{
public:
	//-----Public Methods-----

	Behavior_Flee(const XMLElement& behaviorElement);

	virtual void Update(float deltaTime) override;
	virtual float CalcUtility() override;
	virtual Behavior* Clone() const override;


private:
	//-----Private Methods-----

	void UpdateFleeTargetPosition();

private:

	Vector2 m_fleePosition = Vector2::ZERO;			// The position the actor is fleeing to
	float m_fleeTimer;								// The time left on this flee position
	float m_minSquaredDistanceFromPlayer = 3.f;		// Minimum squared distance the target should be from the player

	static constexpr float TIME_PER_FLEE = 10.f;
};