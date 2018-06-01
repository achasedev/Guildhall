/************************************************************************/
/* File: PlayMode_Default.hpp
/* Author: Andrew Chase
/* Date: March 12th, 2018
/* Description: Class for basic turn management between actors
/************************************************************************/
#pragma once
#include "Game/PlayMode.hpp"

class ActorController;

class PlayMode_Default : public PlayMode
{
public:
	//-----Public Methods-----

	PlayMode_Default();
	~PlayMode_Default();

	// Virtual functions
	virtual void ProcessInput() override;
	virtual void Update() override;
	virtual void RenderWorldSpace() const override;
	virtual void RenderScreenSpace() const override;
	virtual void Exit() override;

	virtual bool IsFinished() const override;


private:
	//-----Private Methods-----

	// For initiating a turn
	void StartActorTurn();
		void AdvanceToNextReadyActor();
		
	// For ending a turn
	void EndActorTurn();


private:
	//-----Private Data-----

	// The current controller for the actor acting this turn
	ActorController*	m_actorController;

};
