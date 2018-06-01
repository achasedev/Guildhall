/************************************************************************/
/* File: GameState_Loading.hpp
/* Author: Andrew Chase
/* Date: February 12th, 2018
/* Description: Class to represent the state when the Game is loading resources
				(Draws a loading screen and loads images/sounds from disk)
/************************************************************************/
#pragma once
#include "Game/GameState.hpp"


class GameState_Loading : public GameState
{
public:
	//-----Public Methods-----

	virtual void ProcessInput() override;
	virtual void Update() override;
	virtual void Render() const override;

	virtual void Enter() override;
	virtual void Leave() override;


private:
	//-----Private Methods-----

	void LoadResources() const;	// CreateOrGets resources


private:
	//-----Private Data-----

	bool m_isSceneRendered = false;	// Flag to start loading resources after the loading screen is drawn

};