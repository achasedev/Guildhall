/************************************************************************/
/* File: PlayState.hpp
/* Author: Andrew Chase
/* Date: October 23rd 2018
/* Description: Base class to control GameState_Playing state
/************************************************************************/
#pragma once
#include "Engine/Core/Time/Stopwatch.hpp"

class GameState_Playing;

class PlayState
{
public:
	//-----Public Methods-----
	
	PlayState(float transitionInTime, float transitionOutTime);
	~PlayState();

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
	//-----Protected Methods-----

	void UpdateWorldAndCamera();


protected:
	//-----Protected Data-----
	
	GameState_Playing* m_gameState = nullptr;
	Stopwatch m_transitionTimer;

	float m_transitionInTime = 0.f;
	float m_transitionOutTime = 0.f;
	
};
