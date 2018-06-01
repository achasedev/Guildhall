/************************************************************************/
/* File: PlayAction_Heal.hpp
/* Author: Andrew Chase
/* Date: March 21st, 2018
/* Description: Action used to heal a target actor
/************************************************************************/
#pragma once
#include <vector>
#include "Game/PlayAction.hpp"
#include "Engine/Math/IntRange.hpp"
#include "Engine/Math/IntVector3.hpp"

class Stopwatch;
class FlyoutText;

class PlayAction_Heal : public PlayAction
{
public:
	//-----Public Methods-----

	PlayAction_Heal(ActorController* actorController);

	// Virtual Methods
	virtual void Setup() override;
	virtual void Start() override;
	virtual void Update() override;
	virtual void RenderWorldSpace() const override;
	virtual void RenderScreenSpace() const override;
	virtual void Exit() override;

	// Callback for when an attack tile has been selected
	void OnTileSelection(bool wasCancelled, const IntVector3& selectedLocation);


private:
	//-----Private Methods-----

	void StartHealStep();
	bool CleanUpFlyouts();
	void FinishHealStep();


private:
	//-----Private Data-----

	// Set in construction
	ActorController*	m_controller;

	// Set in OnTileSelection()
	IntVector3					m_attackSpace;
	Actor*						m_targetActor;

	int							m_healAmount;
	bool						m_healStepStarted;
	std::vector<FlyoutText*>	m_flyouts;

	// Constants
	static const float		HEAL_WAIT_TIME;
	static const IntRange	HEAL_RANGE;
	static const IntRange   HEAL_AMOUNT;
};
