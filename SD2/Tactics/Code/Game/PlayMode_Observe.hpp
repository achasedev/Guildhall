/************************************************************************/
/* File: PlayMode_Observe.hpp
/* Author: Andrew Chase
/* Date: March 12th, 2018
/* Description: Mode for handling free cursor roam by the player
/************************************************************************/
#pragma once
#include "Game/PlayMode.hpp"

class Actor;

class PlayMode_Observe : public PlayMode
{
public:
	//-----Public Methods-----

	PlayMode_Observe(ActorController* controller);

	// Virtual Methods
	virtual void ProcessInput() override;
	virtual void Update() override;
	virtual void RenderWorldSpace() const override;
	virtual void RenderScreenSpace() const override;
	virtual void Exit() override;


private:
	//-----Private Data-----

	Actor* m_hoverActor;	// Actor currently selected by the cursor

};
