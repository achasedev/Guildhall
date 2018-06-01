/************************************************************************/
/* File: PlayAction_Walk.hpp
/* Author: Andrew Chase
/* Date: March 16th, 2018
/* Description: Class to represent an actor's walk action to a target
/************************************************************************/
#pragma once
#include "Game/PlayAction.hpp"
#include "Engine/Math/Vector3.hpp"


class PlayAction_Walk : public PlayAction
{
public:
	//-----Public Methods-----

	PlayAction_Walk(Actor* actor, const Vector3& targetPosition);

	// Virtual Methods
	virtual void Setup() override;
	virtual void Start() override;
	virtual void Update() override;
	virtual void RenderWorldSpace() const override;
	virtual void RenderScreenSpace() const override;
	virtual void Exit() override;


private:
	//-----Private Data-----

	Vector3		m_targetPosition;
	Actor*		m_actor;

};
