/************************************************************************/
/* File: GameState_Loading.hpp
/* Author: Andrew Chase
/* Date: March 24th, 2018
/* Description: Class to represent the state when the Game is loading resources
				(Draws a loading screen and loads images/sounds from disk)
/************************************************************************/
#pragma once
#include "Game/GameStates/GameState.hpp"


class GameState_Loading : public GameState
{
public:
	//-----Public Methods-----

	GameState_Loading();

	virtual void ProcessInput() override;

	virtual bool Enter() override;
	virtual void Update() override;
	virtual bool Leave() override;

	virtual void Render_Enter() const override;
	virtual void Render() const override;
	virtual void Render_Leave() const override;


private:
	//-----Private Methods-----

	void LoadResources() const;	// CreateOrGets resources
	void LoadVoxelResources() const;


private:
	//-----Private Data-----

	bool m_isSceneRendered = false;	// Flag to start loading resources after the loading screen is drawn

};