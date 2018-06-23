/************************************************************************/
/* File: Game.hpp
/* Author: Andrew Chase
/* Date: March 24th, 2018
/* Description: Class used for managing and updating game objects and 
/*              mechanics
/************************************************************************/
#pragma once
#include <map>
#include <string>
#include <vector>
#include "Engine/Math/Vector2.hpp"

class Map;
class Clock;
class GameState;
class RenderScene;
class Player;
class Map;
class GameObject;
class AABB2;
class IntVector2;

enum GameObjectType
{
	GAMEOBJECT_TANK,
	GAMEOBJECT_BULLET,
	NUM_GAMEOBJECT_TYPES
};

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
	static Clock*				GetGameClock();
	static float				GetDeltaTime();

	// Gameplay Accessors
	static Map*			GetMap();
	static RenderScene*	GetRenderScene();
	static Player*		GetPlayer();

	// Gameplay Mutators
	static void InitializePlayer();
	static void InitializeMap(const AABB2& worldBounds, float minHeight, float maxHeight, const IntVector2& chunkLayout, const std::string& fileName);

	static void DeletePlayer();
	static void DeleteMap();


private:
	//-----Private Methods-----
	
	Game();
	~Game();
	Game(const Game& copy) = delete;

	void		CheckToUpdateGameState();


private:
	//-----Private Data-----

	// Framework
	GameState*		m_currentState = nullptr;
	GameState*		m_pendingState = nullptr;
	Clock*			m_gameClock;

	// Gameplay
	Map* m_map = nullptr;
	Player* m_player = nullptr;
	RenderScene* m_renderScene = nullptr;

	static Game* s_instance;	// The singleton Game instance
};
