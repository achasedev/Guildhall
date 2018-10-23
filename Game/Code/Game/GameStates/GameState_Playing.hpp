/************************************************************************/
/* File: GameState_Playing.hpp
/* Author: Andrew Chase
/* Date: July 17th, 2018
/* Description: Class to represent the state when gameplay is active
/************************************************************************/
#pragma once
#include "Game/GameStates/GameState.hpp"
#include "Engine/Core/Time/Stopwatch.hpp"
#include <string>
#include <vector>

enum ePlayState
{
	PLAY_STATE_IDLE,
	PLAY_STATE_WAVE,
	PLAY_STATE_REST,
	PLAY_STATE_PAUSE,
	PLAY_STATE_DEATH,
	PLAY_STATE_VICTORY
};


class World;
class WaveManager;
class PlayState;

class GameState_Playing : public GameState
{
public:
	//-----Public Methods-----

	GameState_Playing();
	~GameState_Playing();

	virtual void ProcessInput() override;
	virtual void Update() override;
	virtual void Render() const override;

	virtual void Enter() override;
	virtual void Leave() override;

	void TransitionToPlayState(PlayState* state);


private:
	//-----Private Data-----

	PlayState* m_currentState = nullptr;
	PlayState* m_transitionState = nullptr;

	bool m_isTransitioning = false;

};
