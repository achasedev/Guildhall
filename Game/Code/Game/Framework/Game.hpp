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
#include "Engine/Math/Transform.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/Time/Stopwatch.hpp"
#include "Engine/Rendering/Core/Renderable.hpp"

class Clock;
class World;
class GameCamera;
class Blackboard;

class Game
{
	
public:
	//-----Public Methods-----

	static void Initialize();
	static void ShutDown();
	
	void ProcessInput();				// Process all input this frame
	void Update();						// Updates all game object states, called each frame
	void Render() const;				// Renders all game objects to screen, called each frame

	static Game*				GetInstance();

	static Clock*				GetGameClock();
	static GameCamera*			GetGameCamera();
	static float				GetDeltaTime();
	static World*				GetWorld();
	static Blackboard*			GetGameConfigBlackboard();


private:
	//-----Private Methods-----
	
	Game();
	~Game();
	Game(const Game& copy) = delete;

	void PopulateGameConfigBlackboard();


private:
	//-----Private Data-----

	Blackboard* m_gameConfigBlackboard = nullptr;
	World*		m_world = nullptr;
	GameCamera*	m_gameCamera = nullptr;
	Clock*		m_gameClock = nullptr;

	static Game* s_instance;			// The singleton Game instance

};
