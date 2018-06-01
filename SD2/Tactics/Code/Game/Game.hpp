/************************************************************************/
/* Project: Protogame2D
/* File: Game.hpp
/* Author: Andrew Chase
/* Date: September 23rd, 2017
/* Bugs: None
/* Description: Class used for managing and updating game objects and 
/*              mechanics
/************************************************************************/
#pragma once
#include <map>
#include <string>
#include "Engine/Math/Vector2.hpp"

class Map;
class BlockDefinition;
class GameState;
class Texture;
class BoardState;
class GameState_Playing;
class OrbitCamera;
class Clock;

class Game
{
	
public:
	//-----Public Methods-----

	static void Initialize();
	static void ShutDown();
	
	void ProcessInput();				// Process all input this frame
	void Update();						// Updates all game object states, called each frame
	void Render() const;				// Renders all game objects to screen, called each frame

	static Game*	GetInstance();
	static void		TransitionToGameState(GameState* newState);

	static GameState*			GetCurrentGameState();
	static GameState_Playing*	GetGameStatePlaying();
	static BoardState*			GetCurrentBoardState();
	static OrbitCamera*			GetGameCamera();
	static Clock*				GetGameClock();
	static float				GetGameDeltaSeconds();


private:
	//-----Private Methods-----
	
	Game();
	~Game();
	Game(const Game& copy) = delete;

	void		CheckToUpdateGameState();


private:
	//-----Private Data-----

	GameState*		m_currentState = nullptr;
	GameState*		m_pendingState = nullptr;
	Clock*			m_gameClock;

	static Game* s_instance;			// The singleton Game instance
};
