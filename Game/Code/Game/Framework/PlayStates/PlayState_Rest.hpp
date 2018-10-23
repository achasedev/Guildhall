/************************************************************************/
/* File: PlayState_Rest.hpp
/* Author: Andrew Chase
/* Date: October 23rd 2018
/* Description: Class representing the state of play between waves
/************************************************************************/
#pragma once
#include "Game/Framework/PlayStates/PlayState_Wave.hpp"

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


public:
	//-----Public Data-----
	

private:
	//-----Private Methods-----
	
	
private:
	//-----Private Data-----
	
	Stopwatch m_restTimer;
	static constexpr float REST_INTERVAL = 5.f;

	static constexpr float REST_TRANSITION_IN_TIME = 1.0f;
	static constexpr float REST_TRANSITION_OUT_TIME = 1.0f;
};
