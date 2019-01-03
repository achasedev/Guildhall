/************************************************************************/
/* File: PlayState_Defeat.cpp
/* Author: Andrew Chase
/* Date: October 24th 2018
/* Description: Implementation of the Defeat PlayState class
/************************************************************************/
#include "Game/Framework/PlayStates/PlayState_Defeat.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Game/Framework/Game.hpp"
#include "Game/GameStates/GameState_MainMenu.hpp"
#include "Game/Framework/World.hpp"


//-----------------------------------------------------------------------------------------------
// Constructor
//
PlayState_Defeat::PlayState_Defeat()
	: PlayState(DEFEAT_TRANSITION_IN_TIME, DEFEAT_TRANSITION_OUT_TIME)
{
}


//-----------------------------------------------------------------------------------------------
// Destructor
//
PlayState_Defeat::~PlayState_Defeat()
{
}


//-----------------------------------------------------------------------------------------------
// Checks for input to return to the main menu
//
void PlayState_Defeat::ProcessInput()
{
	InputSystem* input = InputSystem::GetInstance();
	bool spacePressed = input->WasKeyJustPressed(' ');

	if (spacePressed)
	{
		Game::GetInstance()->TransitionToGameState(new GameState_MainMenu());
	}
	else
	{
		// Check controllers
		for (int i = 0; i < MAX_PLAYERS; ++i)
		{
			XboxController& controller = input->GetController(i);

			if (controller.WasButtonJustPressed(XBOX_BUTTON_START) || controller.WasButtonJustPressed(XBOX_BUTTON_A))
			{
				Game::GetInstance()->TransitionToGameState(new GameState_MainMenu());
			}
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Enter - does nothing for now
//
bool PlayState_Defeat::Enter()
{
	if (m_transitionTimer.HasIntervalElapsed())
	{
		Game::UpdateLeaderboardWithCurrentScore();
		return true;
	}

	return false;
}


//-----------------------------------------------------------------------------------------------
// Update - doesn't update anything
//
void PlayState_Defeat::Update()
{
	// Nothing!
}


//-----------------------------------------------------------------------------------------------
// Leave - does nothing for now
//
bool PlayState_Defeat::Leave()
{
	return m_transitionTimer.HasIntervalElapsed();
}


//-----------------------------------------------------------------------------------------------
// Renders the enter transition of the defeat state
//
void PlayState_Defeat::Render_Enter() const
{
	Game::GetWorld()->DrawToGrid();
}


//-----------------------------------------------------------------------------------------------
// Renders the defeat state screen and text
//
void PlayState_Defeat::Render() const
{
	Game::GetWorld()->DrawToGrid();
}


//-----------------------------------------------------------------------------------------------
// Renders the leave transition of the defeat state
//
void PlayState_Defeat::Render_Leave() const
{
	Game::GetWorld()->DrawToGrid();

}
