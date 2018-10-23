/************************************************************************/
/* File: PlayState_Wave.hpp
/* Author: Andrew Chase
/* Date: October 23rd 2018
/* Description: Class to represent the state of play during a wave
/************************************************************************/
#pragma once
#include "Game/Framework/PlayStates/PlayState.hpp"

class PlayState_Wave : public PlayState
{
public:
	//-----Public Methods-----
	
	PlayState_Wave();
	~PlayState_Wave();

	virtual void ProcessInput() override;

	virtual bool Enter() override;
	virtual void Update() override;
	virtual bool Leave() override;
	
	virtual void Render_Enter() const override;
	virtual void Render() const override;
	virtual void Render_Leave() const override;

	
private:
	//-----Private Data-----
	
	static constexpr float WAVE_TRANSITION_IN_TIME = 1.0f;
	static constexpr float WAVE_TRANSITION_OUT_TIME = 1.0f;

};
