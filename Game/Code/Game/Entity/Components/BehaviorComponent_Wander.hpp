/************************************************************************/
/* File: BehaviorComponent_Wander.hpp
/* Author: Andrew Chase
/* Date: March 29th 2019
/* Description: Behavior where the entity moves to random targets
/************************************************************************/
#pragma once
#include "Game/Entity/Components/BehaviorComponent.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Time/Stopwatch.hpp"


enum eWanderState
{
	WANDER_STATE_MOVING,
	WANDER_STATE_WAIT
};

class BehaviorComponent_Wander : public BehaviorComponent
{
public:
	//-----Public Methods-----

	BehaviorComponent_Wander() {}
	~BehaviorComponent_Wander() {}

	virtual void				Update() override;
	virtual BehaviorComponent*	Clone() const override;


private:
	//-----Private Methods-----

	void UpdateMove();
	void UpdateWait();


private:
	//-----Private Data-----

	eWanderState m_state = WANDER_STATE_WAIT;
	Stopwatch m_stateTimer;
	Vector2 m_targetPosition;
	
	static constexpr float WANDER_MAX_MOVE_TIME = 10.f;
	static constexpr float WANDER_BASE_WAIT_TIME = 3.f;
	static constexpr float WANDER_WAIT_DEVIANCE = 1.f;
	static constexpr float WANDER_ROTATION_SPEED = 45.f;

};
