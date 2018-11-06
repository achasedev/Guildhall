/************************************************************************/
/* File: GameState.cpp
/* Author: Andrew Chase
/* Date: March 24th, 2018
/* Description: Implementation of the GameState abstract class (NONE)
/************************************************************************/
#include "Game/GameStates/GameState.hpp"

GameState::GameState(float transitionInTime, float transitionOutTime)
	: m_transitionInTime(transitionInTime), m_transitionOutTime(transitionOutTime)
{
}

GameState::~GameState()
{
}

void GameState::StartEnterTimer()
{
	m_transitionTimer.SetInterval(m_transitionInTime);
}

void GameState::StartLeaveTimer()
{
	m_transitionTimer.SetInterval(m_transitionOutTime);
}
