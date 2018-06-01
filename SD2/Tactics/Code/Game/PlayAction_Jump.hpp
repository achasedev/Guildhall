/************************************************************************/
/* File: PlayAction_Jump.hpp
/* Author: Andrew Chase
/* Date: March 16th, 2018
/* Description: Class represent an action of a player jumping to an
				adjacent block of different elevation
/************************************************************************/
#pragma once
#include "Game/PlayAction.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/Vector3.hpp"

class Actor;
class Stopwatch;

class PlayAction_Jump : public PlayAction
{
public:
	//-----Public Methods-----

	PlayAction_Jump(Actor* actor, const Vector3& destination);

	// Virtual Methods
	virtual void Setup() override;
	virtual void Start() override;
	virtual void Update() override;
	virtual void RenderWorldSpace() const override;
	virtual void RenderScreenSpace() const override;
	virtual void Exit() override;


private:
	//-----Private Data-----

	Actor* m_actor;

	// Trajectory
	Vector3		m_startPosition;
	Vector3		m_endPosition;

	Vector2		m_launchVelocity;
	float		m_launchDuration;
	Stopwatch*	m_stopwatch;

	static const float GRAVITY;
};
