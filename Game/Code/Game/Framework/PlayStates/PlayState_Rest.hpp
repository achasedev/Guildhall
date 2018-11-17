/************************************************************************/
/* File: PlayState_Rest.hpp
/* Author: Andrew Chase
/* Date: October 23rd 2018
/* Description: Class representing the state of play between stages
/************************************************************************/
#pragma once
#include "Game/Framework/PlayStates/PlayState_Stage.hpp"

class World;

class PlayState_Rest : public PlayState
{
public:
	//-----Public Methods-----
	
	PlayState_Rest();
	~PlayState_Rest();

	virtual void ProcessInput() override;

	virtual bool Enter() override;
	virtual void Update() override;
	virtual bool Leave() override;
	
	virtual void Render_Enter() const override;
	virtual void Render() const override;
	virtual void Render_Leave() const override;
	
	
private:
	//-----Private Data-----
	
	// Timer for controlling when to move to the next stage state
	//Stopwatch				m_restTimer;
	
	IntVector3 m_transitionOffset = IntVector3::ZERO;
	World* m_worldToTransitionTo = nullptr;

	eTransitionEdge m_edgeToEnter;
	eTransitionEdge m_edgeToExit;

	int m_decrementCount = 0;

	static constexpr float REST_INTERVAL = 5.f;
	static constexpr float REST_TRANSITION_IN_TIME = 1.0f;
	static constexpr float REST_TRANSITION_OUT_TIME = 2.0f;

};
