/************************************************************************/
/* File: PlayAction_Arrow.hpp
/* Author: Andrew Chase
/* Date: March 19th, 2018
/* Description: Action used for when an archer shoots an arrow
/************************************************************************/
#pragma once
#include <vector>
#include "Game/PlayAction.hpp"
#include "Engine/Math/IntVector3.hpp"

class Stopwatch;
class FlyoutText;

class PlayAction_Arrow : public PlayAction
{
public:
	//-----Public Methods-----

	PlayAction_Arrow(ActorController* actorController);

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

	void CalculateTrajectory();

	void UpdateTrajectory();
	void StartDamageStep();
	bool CleanUpFlyouts();
	void FinishDamageStep();

	float GetPositionalBlockChance() const;
	float GetPositionalCritChance() const;


private:
	//-----Private Data-----

	// Set in construction
	ActorController*	m_controller;

	// Set in OnTileSelection()
	IntVector3			m_attackSpace;
	Actor*				m_targetActor;

	// Set during Start()
	Vector3				m_startPosition;
	Vector3				m_endPosition;
	Vector3				m_launchDirectionXZ;	// Direction in the world xz/map xy plane
	float				m_launchDuration;		// Time it takes from launch to landing at target
	Vector2				m_launchVelocity;

	Stopwatch*			m_stopwatch;

	// Set during Update()
	Vector3						m_projectilePosition;

	int							m_damageAmount;
	bool						m_damageStepStarted;
	std::vector<FlyoutText*>	m_flyouts;

	// Constants
	static const float		GRAVITY;
	static const float		LAUNCH_SPEED;
	static const float		ARROW_WAIT_TIME;
	static const int		NUM_STEP_SAMPLES;
	static const IntRange	ARROW_DAMAGE_RANGE;

};
