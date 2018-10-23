/************************************************************************/
/* File: Game.hpp
/* Author: Andrew Chase
/* Date: March 24th, 2018
/* Description: Class used for managing and updating game objects and 
/*              mechanics
/************************************************************************/
#pragma once
#include <vector>
#include "Engine/Math/Vector2.hpp"

#define MAX_PLAYERS (4)

class Clock;
class World;
class WaveManager;
class GameCamera;
class Player;
class GameState;

class Game
{
	
public:
	friend class GameState_Playing;

	//-----Public Methods-----

	static void Initialize();
	static void ShutDown();
	
	void ProcessInput();				// Process all input this frame
	void Update();						// Updates all game object states, called each frame
	void Render() const;				// Renders all game objects to screen, called each frame

	GameState* GetGameState() const;

	static Game*	GetInstance();
	static void		TransitionToGameState(GameState* newState);

	static Clock*				GetGameClock();
	static GameCamera*			GetGameCamera();
	static float				GetDeltaTime();
	static World*				GetWorld();
	static Player**				GetPlayers();
	static WaveManager*			GetWaveManager();


private:
	//-----Private Methods-----
	
	Game();
	~Game();
	Game(const Game& copy) = delete;

	void		CheckToUpdateGameState();
	void		UpdateCameraOnInput();


private:
	//-----Private Data-----

	GameState*					m_currentState = nullptr;
	GameState*					m_pendingState = nullptr;
	Clock*						m_gameClock = nullptr;
	World*						m_world = nullptr;
	WaveManager*				m_waveManager = nullptr;

	Player*						m_players[MAX_PLAYERS];

	// Camera
	GameCamera*					m_gameCamera = nullptr;

	static Game* s_instance;			// The singleton Game instance

};
