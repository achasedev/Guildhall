/************************************************************************/
/* File: GameState_Ready.hpp
/* Author: Andrew Chase
/* Date: June 1st, 2018
/* Description: Class to represent the state where player can make decisions
/*				before play starts
/************************************************************************/
#pragma once

#include "Game/GameState.hpp"
#include "Engine/Math/AABB2.hpp"

class GameState_Ready : public GameState
{
public:
	//-----Public Methods-----

	virtual void ProcessInput() override;
	virtual void Update() override;
	virtual void Render() const override;

	virtual void Enter() override;
	virtual void Leave() override;

private:
	//-----Private Data-----

	AABB2 m_textBoxBounds;

};
