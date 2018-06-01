/************************************************************************/
/* File: Game.cpp
/* Author: Andrew Chase
/* Date: January 23rd, 2017
/* Bugs: None
/* Description: Game class used to draw the triangle to screen
/************************************************************************/
#include "Game.hpp"
#include "Game/GameCommon.hpp"
#include "Game/GameState.hpp"
#include "Game/GameState_Loading.hpp"
#include "Game/GameState_MainMenu.hpp"
#include "Game/GameState_Playing.hpp"
#include "Game/Map.hpp"
#include "Game/BlockDefinition.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/Command.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/Clock.hpp"

// The singleton instance
Game* Game::s_instance = nullptr;

//-----------------------------------------------------------------------------------------------
// Default constructor, initialize any game members here (private)
//
Game::Game()
	: m_currentState(new GameState_Loading())
{
	m_gameClock = new Clock(Clock::GetMasterClock());
	BlockDefinition::LoadDefinitions();
}


//-----------------------------------------------------------------------------------------------
// Destructor - clean up any allocated members (private)
//
Game::~Game()
{
}


//-----------------------------------------------------------------------------------------------
// Constructs the singleton game instance
//
void Game::Initialize()
{
	GUARANTEE_OR_DIE(s_instance == nullptr, "Error: Game::Initialize called when a Game instance already exists.");
	s_instance = new Game();
}


//-----------------------------------------------------------------------------------------------
// Deletes the singleton instance
//
void Game::ShutDown()
{
	delete s_instance;
	s_instance = nullptr;
}


//-----------------------------------------------------------------------------------------------
// Checks for input received this frame and updates states accordingly
//
void Game::ProcessInput()
{
	// Reload shader check
	if (InputSystem::GetInstance()->WasKeyJustPressed(InputSystem::KEYBOARD_F9))
	{
		Renderer::GetInstance()->ReloadShaders();
	}

	m_currentState->ProcessInput();
}


//-----------------------------------------------------------------------------------------------
// Update the movement variables of every entity in the world, as well as update the game state
// based on the input this frame
//
void Game::Update()
{
	// Check for state change
	CheckToUpdateGameState();

	// Update the current state
	m_currentState->Update();
}


//-----------------------------------------------------------------------------------------------
// Draws to screen
//
void Game::Render() const
{
	m_currentState->Render();
}


//-----------------------------------------------------------------------------------------------
// Sets the pending state flag to the one given, so the next frame the game will switch to the
// given state
//
void Game::TransitionToGameState(GameState* newState)
{
	s_instance->m_pendingState = newState;
}


//-----------------------------------------------------------------------------------------------
// Returns the current GameState of the singleton Game instance
//
GameState* Game::GetCurrentGameState()
{
	return s_instance->m_currentState;
}


//-----------------------------------------------------------------------------------------------
// Returns the current GameState of the game instance as a GameState_Playing
//
GameState_Playing* Game::GetGameStatePlaying()
{
	return dynamic_cast<GameState_Playing*>(s_instance->m_currentState);
}


//-----------------------------------------------------------------------------------------------
// Returns the current board state of the game
//
BoardState* Game::GetCurrentBoardState()
{
	// BoardState only exists if we're in the play mode (should only be called then anyways...)
	GameState_Playing* playState = dynamic_cast<GameState_Playing*>(s_instance->m_currentState);

	if (playState != nullptr)
	{
		return playState->GetBoardState();
	}

	return nullptr;
}


//-----------------------------------------------------------------------------------------------
// Returns the single camera used by the game
//
OrbitCamera* Game::GetGameCamera()
{
	GameState_Playing* playState = dynamic_cast<GameState_Playing*>(s_instance->m_currentState);

	if (playState != nullptr)
	{
		return playState->GetGameCamera();
	}

	return nullptr;
}


//-----------------------------------------------------------------------------------------------
// Returns the game clock
//
Clock* Game::GetGameClock()
{
	return s_instance->m_gameClock;
}


//-----------------------------------------------------------------------------------------------
// Returns the frame time for the game clock
//
float Game::GetGameDeltaSeconds()
{
	return s_instance->m_gameClock->GetFrameSeconds();
}


//-----------------------------------------------------------------------------------------------
// Returns the singleton Game instance
//
Game* Game::GetInstance()
{
	return s_instance;
}


//-----------------------------------------------------------------------------------------------
// Checks if there is a pending 
//
void Game::CheckToUpdateGameState()
{
	// Have a state pending
	if (m_pendingState != nullptr)
	{
		if (m_currentState != nullptr)
		{
			// Leave and destroy current
			m_currentState->Leave();
			delete m_currentState;
		}

		// Set new as current
		m_currentState = m_pendingState;
		m_pendingState = nullptr;

		// Enter the new state
		m_currentState->Enter();
	}
}
