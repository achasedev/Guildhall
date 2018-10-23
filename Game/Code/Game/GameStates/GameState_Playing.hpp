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
class SpawnManager;

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


private:
	//-----Private Methods-----

	void UpdateCameraOnInput();

	
	void Update_Idle();
	void Update_Wave();
	void Update_Rest();
	void Update_Pause();
	void Update_Victory();
	void Update_Death();

	void Render_Idle();
	void Render_Wave();
	void Render_Rest();
	void Render_Pause();
	void Render_Victory();
	void Render_Death();

	void TransitionToState(ePlayState state);


private:
	//-----Private Data-----

	ePlayState m_state = PLAY_STATE_IDLE;
	ePlayState m_transition = PLAY_STATE_IDLE;

	Stopwatch m_restTimer;

	bool m_cameraEjected = false;

	SpawnManager*	m_spawnManager = nullptr;

	static constexpr float CAMERA_ROTATION_SPEED = 45.f;
	static constexpr float CAMERA_TRANSLATION_SPEED = 10.f;
	static constexpr float REST_INTERVAL = 5.f;

};
