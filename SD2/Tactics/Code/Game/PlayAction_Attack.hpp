/************************************************************************/
/* File: PlayAction_Attack.hpp
/* Author: Andrew Chase
/* Date: March 12th, 2018
/* Description: Action used for the damage step of an actor attack
/************************************************************************/
#pragma once
#include <vector>
#include "Game/PlayAction.hpp"
#include "Engine/Math/IntVector3.hpp"

class FlyoutText;

class PlayAction_Attack : public PlayAction
{
public:
	//-----Public Methods-----

	PlayAction_Attack(ActorController* actorController);

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

	void StartDamageStep();
	bool CleanUpFlyouts();
	void FinishDamageStep();

	float GetPositionalBlockChance() const;
	float GetPositionalCritChance() const;


private:
	//-----Private Data-----

	IntVector3			m_attackSpace;
	Actor*				m_defendingActor;
	int					m_damageAmount;

	ActorController*	m_controller;

	std::vector<FlyoutText*> m_flyoutTexts;

};
