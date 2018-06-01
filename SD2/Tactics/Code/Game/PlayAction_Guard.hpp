/************************************************************************/
/* File: PlayAction_Guard.hpp
/* Author: Andrew Chase
/* Date: March 21st, 2018
/* Description: Action used for increasing block rate on an actor
/************************************************************************/
#pragma once
#include <vector>
#include "Game/PlayAction.hpp"

class FlyoutText;

class PlayAction_Guard : public PlayAction
{
public:
	//-----Public Methods-----

	PlayAction_Guard(ActorController* actorController);

	// Virtual Methods
	virtual void Setup() override;
	virtual void Start() override;
	virtual void Update() override;
	virtual void RenderWorldSpace() const override;
	virtual void RenderScreenSpace() const override;
	virtual void Exit() override;


private:
	//-----Private Methods-----

	bool	CleanUpFlyouts();


private:
	//-----Private Data-----

	ActorController*	m_controller;
	std::vector<FlyoutText*> m_flyoutTexts;

	// Statics
	static const float GUARD_BLOCK_BONUS;
	static const float GUARD_WAIT_TIME;
};
