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
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/Time/Stopwatch.hpp"
#include "Engine/Rendering/Meshes/Mesh.hpp"

class Clock;
class Camera;
class GameState;

enum eGameStateState
{
	GAME_STATE_TRANSITIONING_IN,
	GAME_STATE_UPDATING,
	GAME_STATE_TRANSITIONING_OUT
};

class Game
{
	
public:
	friend class GameState_Playing;
	friend class GameState_Loading;

	//-----Public Methods-----

	static void Initialize();
	static void ShutDown();
	
	void ProcessInput();				// Process all input this frame
	void Update();						// Updates all game object states, called each frame
	void Render() const;				// Renders all game objects to screen, called each frame

	// Audio
	bool SetupDSPForTrack(const std::string& audioPath);


	// Accessors
	GameState*					GetGameState() const;
	static Game*				GetInstance();
	static Clock*				GetGameClock();
	static Camera*				GetGameCamera();
	static float				GetDeltaTime();

	// Mutators
	static void					TransitionToGameState(GameState* newState);


private:
	//-----Private Methods-----
	
	Game();
	~Game();
	Game(const Game& copy) = delete;

	void UpdateBarMesh();


private:
	//-----Private Data-----

	bool			m_doneLoading = false;
	GameState*		m_currentState = nullptr;
	GameState*		m_transitionState = nullptr;
	eGameStateState m_gameStateState = GAME_STATE_TRANSITIONING_IN;

	Camera*			m_gameCamera = nullptr;
	Clock*			m_gameClock = nullptr;

	mutable Mesh			m_barMesh;

	int m_numChannels = 2;
	int m_numSegmentsTotal = 0;
	int m_numSegmentsBeingAnalyzed = 0;
	int m_zeroCount = 0;
	float m_maxValue = 0.f;
	float m_sumOfValues = 0.f;

	static constexpr int FFT_WINDOW_SIZE = 1024;
	static constexpr int FFT_WINDOW_FRACTION_DIVISOR = 2;

	static constexpr float FONT_HEIGHT = 30.f;
	static Game* s_instance;			// The singleton Game instance

};
