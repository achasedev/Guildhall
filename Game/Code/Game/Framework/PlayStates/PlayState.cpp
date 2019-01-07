/************************************************************************/
/* File: PlayState.cpp
/* Author: Andrew Chase
/* Date: October 24th 2018
/* Description: Implementation of the PlayState abstract class
/************************************************************************/
#include "Game/Framework/Game.hpp"
#include "Game/GameStates/GameState_Playing.hpp"
#include "Game/Framework/PlayStates/PlayState.hpp"
#include "Game/Framework/World.hpp"
#include "Game/Framework/GameCamera.hpp"
#include "Engine/Input/InputSystem.hpp"

//-----------------------------------------------------------------------------------------------
// Constructor
//
PlayState::PlayState(float transitionInTime, float transitionOutTime)
	: m_transitionInTime(transitionInTime)
	, m_transitionOutTime(transitionOutTime)
	, m_transitionTimer(Game::GetGameClock())
	, m_gameState(dynamic_cast<GameState_Playing*>(Game::GetInstance()->GetGameState()))
{
}


//-----------------------------------------------------------------------------------------------
// Destructor
//
PlayState::~PlayState()
{
}


//-----------------------------------------------------------------------------------------------
// Starts the transition timer for the state's enter
//
void PlayState::StartEnterTimer()
{
	m_transitionTimer.SetInterval(m_transitionInTime);
	m_state = TRANSITION_STATE_ENTERING;
}


//-----------------------------------------------------------------------------------------------
// Called when the state first begins updating
//
void PlayState::StartUpdating()
{
	m_transitionTimer.Reset();
	m_state = TRANSITION_STATE_UPDATING;
}


//-----------------------------------------------------------------------------------------------
// Starts the transition timer for the state's leave
//
void PlayState::StartLeaveTimer()
{
	m_transitionTimer.SetInterval(m_transitionOutTime);
	m_state = TRANSITION_STATE_LEAVING;
}


//-----------------------------------------------------------------------------------------------
// Updates the world (all entities and particles) and the game camera to focus on the players, as
// per normal
//
void PlayState::UpdateWorldAndCamera()
{
	// World
	Game::GetWorld()->Update();

	// Camera
	GameCamera* camera = Game::GetGameCamera();

	if (!camera->IsEjected())
	{
		//Game::GetGameCamera()->UpdatePositionBasedOnPlayers();
		Game::GetGameCamera()->LookAtGridCenter();
	}
}


//-----------------------------------------------------------------------------------------------
// Returns the index of the player that pressed pause if pause was pressed by keyboard or a controller
//
int CheckForPause()
{
	InputSystem* input = InputSystem::GetInstance();

	// Check keyboard first
	if (input->WasKeyJustPressed(InputSystem::KEYBOARD_ESCAPE))
	{
		return 0;
	}

	// Check controllers
	for (int i = 0; i < MAX_PLAYERS; ++i)
	{
		if (Game::GetPlayers()[i] != nullptr)
		{
			XboxController& controller = input->GetController(i);
			if (controller.WasButtonJustPressed(XBOX_BUTTON_START))
			{
				return i;
			}
		}
	}

	// No pause pressed
	return -1;
}