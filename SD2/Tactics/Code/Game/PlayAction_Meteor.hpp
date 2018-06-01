/************************************************************************/
/* File: PlayAction_Meteor.hpp
/* Author: Andrew Chase
/* Date: March 19th, 2018
/* Description: Action used by mages to drop an AoE meteor attack
/************************************************************************/
#pragma once
#include "Game/PlayAction.hpp"
#include "Engine/Math/IntVector3.hpp"

// Struct to track damage to a single target
struct MeteorDamageInfo
{
	MeteorDamageInfo(int amount, bool wasCrit, Actor* actorHit)
		: m_amount(amount), m_wasCrit(wasCrit), m_actorHit(actorHit)
	{}
	int		m_amount;
	bool	m_wasCrit;
	Actor*	m_actorHit;
};

class Stopwatch;
class FlyoutText;

class PlayAction_Meteor : public PlayAction
{
public:
	//-----Public Methods-----

	PlayAction_Meteor(ActorController* actorController);

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

	void CreateFlyoutForResult(MeteorDamageInfo damageInfo);


private:
	//-----Private Data-----

	IntVector3			m_targetSpace;
	Vector3				m_meteorPosition;	// For rendering
	Stopwatch*			m_stopwatch;
	ActorController*	m_controller;

	bool							m_damageStepStarted;
	std::vector<MeteorDamageInfo>	m_damageResults;
	std::vector<FlyoutText*>		m_flyouts;

	static const int		METEOR_AOE_RANGE;
	static const float		METEOR_DROP_SPEED;
	static const float		METEOR_DROP_INTERVAL;
	static const float		METEOR_WAIT_TIME;
	static const IntRange	METEOR_CAST_RANGE;
	static const IntRange	METEOR_DAMAGE_RANGE;
	static const Vector3	METEOR_SPAWN_OFFSET;

};
