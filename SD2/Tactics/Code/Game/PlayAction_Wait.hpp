/************************************************************************/
/* File: PlayAction_Wait.hpp
/* Author: Andrew Chase
/* Date: March 12th, 2018
/* Description: Action used ending an actor's turn
/************************************************************************/
#pragma once
#include "Game/PlayAction.hpp"

class Actor;

class PlayAction_Wait : public PlayAction
{
public:
	//-----Public Methods-----

	PlayAction_Wait(ActorController* controller);

	// Virtual Methods
	virtual void Setup() override;
	virtual void Start() override;
	virtual void Update() override;
	virtual void RenderWorldSpace() const override;
	virtual void RenderScreenSpace() const override;
	virtual void Exit() override;


private:
	//-----Private Data-----
	
	ActorController* m_controller;
	const static float		ACTOR_MIN_WAIT_TIME;

};
