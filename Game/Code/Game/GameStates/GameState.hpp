/************************************************************************/
/* File: GameState.hpp
/* Author: Andrew Chase
/* Date: March 24th, 2018
/* Description: Abstract class to represent a given Game's state
/************************************************************************/
#pragma once
#include "Game/Framework/Game.hpp"
#include "Engine/Core/Time/Stopwatch.hpp"

class GameState
{
public:
	//-----Public Methods-----

	GameState(float transitionInTime, float transitionOutTime);
	~GameState();

	virtual void ProcessInput() = 0;

	virtual bool Enter() = 0;
	virtual void Update() = 0;
	virtual bool Leave() = 0;

	virtual void Render_Enter() const = 0;
	virtual void Render() const = 0;
	virtual void Render_Leave() const = 0;

	void StartEnterTimer();
	void StartLeaveTimer();


protected:
	//-----Protected Data-----

	Stopwatch m_transitionTimer;

	float m_transitionInTime = 0.f;
	float m_transitionOutTime = 0.f;

};
