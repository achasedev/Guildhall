/************************************************************************/
/* File: PlayMode_Victory.hpp
/* Author: Andrew Chase
/* Date: March 12th, 2018
/* Description: Mode for handling the victory state of the game
/************************************************************************/
#pragma once
#include "Game/PlayMode.hpp"

class Stopwatch;

class PlayMode_Victory : public PlayMode
{
public:
	//-----Public Methods-----

	PlayMode_Victory();

	// Virtual Methods
	virtual void Start() override;
	virtual void ProcessInput() override;
	virtual void Update() override;
	virtual void RenderWorldSpace() const override;
	virtual void RenderScreenSpace() const override;
	virtual void Exit() override;


private:
	//-----Private Data-----

	Stopwatch* m_stopwatch;
};
