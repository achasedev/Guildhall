/************************************************************************/
/* File: PlayMode_Observe.cpp
/* Author: Andrew Chase
/* Date: March 12th, 2018
/* Description: Mode for handling Actor turn menu state and input
/************************************************************************/
#include "Game/Map.hpp"
#include "Game/Game.hpp"
#include "Game/Actor.hpp"
#include "Game/BoardState.hpp"
#include "Game/GameCommon.hpp"
#include "Game/PlayMode_Observe.hpp"
#include "Game/GameState_Playing.hpp"
#include "Engine/Input/InputSystem.hpp"

//-----------------------------------------------------------------------------------------------
// Only constructor - takes the controller of the currently acting actor
//
PlayMode_Observe::PlayMode_Observe(ActorController* controller)
	: PlayMode(controller)
{
}


//-----------------------------------------------------------------------------------------------
// Checks for input
//
void PlayMode_Observe::ProcessInput()
{
	// Update the cursor
	PlayMode::ProcessInput();
	
	// Check to exit the mode
	XboxController& controller = InputSystem::GetPlayerOneController();
	if (controller.WasButtonJustPressed(XBOX_BUTTON_B))
	{
		m_isFinished = true;
	}
}


//-----------------------------------------------------------------------------------------------
// Update
//
void PlayMode_Observe::Update()
{
	// Translate the camera to the cursor position
	GameState_Playing* playState = Game::GetGameStatePlaying();
	Map* map = Game::GetCurrentBoardState()->GetMap();
	playState->TranslateCamera(map->MapCoordsToWorldPosition(m_cursorCoords));
}


//-----------------------------------------------------------------------------------------------
// Draws this mode's world space items
//
void PlayMode_Observe::RenderWorldSpace() const
{
	// Render the cursor
	PlayMode::RenderWorldSpace();
}


//-----------------------------------------------------------------------------------------------
// Draws screen space items
//
void PlayMode_Observe::RenderScreenSpace() const
{
	PlayMode::RenderScreenSpace();
}


//-----------------------------------------------------------------------------------------------
// Called before deletion of this mode
//
void PlayMode_Observe::Exit()
{
}
