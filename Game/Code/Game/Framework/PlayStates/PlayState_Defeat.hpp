/************************************************************************/
/* File: PlayState_Defeat.hpp
/* Author: Andrew Chase
/* Date: October 23rd 2018
/* Description: Class to represent the state of play when all players are dead
/************************************************************************/
#pragma once
#include "Game/Framework/PlayStates/PlayState.hpp"

class PlayState_Defeat : public PlayState
{
public:
	//-----Public Methods-----

	PlayState_Defeat();
	~PlayState_Defeat();

	virtual void ProcessInput() override;

	virtual bool Enter() override;
	virtual void Update() override;
	virtual bool Leave() override;

	virtual void Render_Enter() const override;
	virtual void Render() const override;
	virtual void Render_Leave() const override;


private:
	//-----Private Data-----

	static constexpr float DEFEAT_TRANSITION_IN_TIME = 2.0f;
	static constexpr float DEFEAT_TRANSITION_OUT_TIME = 2.0f;

};
