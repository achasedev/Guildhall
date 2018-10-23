/************************************************************************/
/* File: PlayState_Victory.hpp
/* Author: Andrew Chase
/* Date: October 23rd 2018
/* Description: Class representing the state of play when the players win
/************************************************************************/
#pragma once
#include "Game/Framework/PlayStates/PlayState.hpp"

class PlayState_Victory : public PlayState
{
public:
	//-----Public Methods-----
	
	PlayState_Victory();
	~PlayState_Victory();

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
	
	static constexpr float VICTORY_TRANSITION_IN_TIME = 1.0f;
	static constexpr float VICTORY_TRANSITION_OUT_TIME = 1.0f;

};
