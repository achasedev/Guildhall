/************************************************************************/
/* File: GameState_Ready.hpp
/* Author: Andrew Chase
/* Date: June 1st, 2018
/* Description: Class to represent the state where player can make decisions
/*				before play starts
/************************************************************************/
#pragma once

#include "Game/GameStates/GameState.hpp"
#include "Engine/Math/AABB2.hpp"

class GameState_Ready : public GameState
{
public:
	//-----Public Methods-----

	GameState_Ready();

	virtual void ProcessInput() override;

	virtual bool Enter() override;
	virtual void Update() override;
	virtual bool Leave() override;

	virtual void Render_Enter() const override;
	virtual void Render() const override;
	virtual void Render_Leave() const override;

private:
	//-----Private Data-----

	AABB2 m_textBoxBounds;

};
