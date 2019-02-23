/************************************************************************/
/* File: BehaviorComponent_Smash.hpp
/* Author: Andrew Chase
/* Date: February 23rd 2019
/* Description: Behavior for an enemy that moves over players and smashes down
/************************************************************************/
#pragma once
#include "Game/Entity/Components/BehaviorComponent.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Core/Time/Stopwatch.hpp"

enum eSmashState
{
	STATE_WAITING_ON_GROUND,
	STATE_MOVING_TO_HOVER_TARGET,
	STATE_HOVERING,
	STATE_SMASHING
};

class BehaviorComponent_Smash : public BehaviorComponent
{
public:
	//-----Public Methods-----

	BehaviorComponent_Smash();

	virtual void				Initialize(AnimatedEntity* owningEntity) override;
	virtual void				Update() override;
	virtual BehaviorComponent*	Clone() const override;


private:
	//-----Private Methods-----

	void UpdateWaitingOnGround();
	void UpdateMovingToHoverTarget();
	void UpdateHovering();
	void UpdateSmashing();


private:
	//-----Private Data-----

	eSmashState m_state = STATE_WAITING_ON_GROUND;
	Vector3 m_hoverTarget;
	Vector3 m_hoverDirection;
	Stopwatch m_waitTimer;

	int m_damageOnTouch = 1;
	int m_damageOnSmash = 3;
	float m_smashKnockBackMagnitude = 30.f;

	static constexpr float HOVER_HEIGHT_ABOVE_PLAYER = 30.f;
	static constexpr float HOVER_WAIT_TIME = 1.0f;
	static constexpr float GROUND_WAIT_TIME = 2.0f;
	static constexpr float HOVER_TRANSLATION_SPEED = 150.f;
	static constexpr float TRANSLATION_THRESHOLD_ACCURACY = 2.0f;
};