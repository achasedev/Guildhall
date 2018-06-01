/************************************************************************/
/* File: ActorController.hpp
/* Author: Andrew Chase
/* Date: March 12th, 2017
/* Description: Class to control an actor during a turn
/************************************************************************/
#pragma once
#include <vector>
#include "Game/Actor.hpp"

class PlayAction;

class ActorController
{
public:
	//-----Public Methods-----

	ActorController(Actor& actor);
	~ActorController();

	// Update
	void Update();
	void OnMenuSelection(bool wasCancelled, eActionName selectedAction);

	// Mutators
	void SetControllerFlag(eActionClass actionClass, bool newState);

	// Producers
	bool WasActionClassUsed(eActionClass actionClass) const;
	bool HasTurnEnded() const;

	// Accessors
	Actor* GetActor();


private:
	//-----Private Methods-----

	bool CheckForVictory();


private:
	//-----Private Data-----

	// Actor currently being controlled by the controller
	Actor& m_actor;

	// Action currently selected by the controller, not enqueued yet
	PlayAction* m_selectedAction;

	// Per-turn flags
	bool m_controllerFlags[NUM_ACTION_CLASSES];
};
