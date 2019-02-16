/************************************************************************/
/* File: PlayState_Stage.hpp
/* Author: Andrew Chase
/* Date: October 23rd 2018
/* Description: Class to represent the state of play during a stage
/************************************************************************/
#pragma once
#include "Game/Framework/PlayStates/PlayState.hpp"

class PlayState_Stage : public PlayState
{
public:
	//-----Public Methods-----
	
	PlayState_Stage();
	~PlayState_Stage();

	virtual void ProcessInput() override;

	virtual bool Enter() override;
	virtual void Update() override;
	virtual bool Leave() override;
	
	virtual void Render_Enter() const override;
	virtual void Render() const override;
	virtual void Render_Leave() const override;

	
private:
	//-----Private Data-----
	
	static constexpr float STAGE_TRANSITION_IN_TIME = 1.0f;
	static constexpr float STAGE_TRANSITION_OUT_TIME = 0.5f;

};
