/************************************************************************/
/* File: PlayState_Victory.cpp
/* Author: Andrew Chase
/* Date: October 25th 2018
/* Description: Implementation of the Victory PlayState
/************************************************************************/
#include "Game/Framework/PlayStates/PlayState_Victory.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Game/Framework/Game.hpp"
#include "Game/GameStates/GameState_MainMenu.hpp"
#include "Game/Framework/World.hpp"

// For debug printing the state
#include "Engine/Core/Window.hpp"
#include "Engine/Core/Utility/StringUtils.hpp"
#include "Engine/Rendering/DebugRendering/DebugRenderSystem.hpp"


//-----------------------------------------------------------------------------------------------
// Constructor
//
PlayState_Victory::PlayState_Victory()
	: PlayState(VICTORY_TRANSITION_IN_TIME, VICTORY_TRANSITION_OUT_TIME)
{
}


//-----------------------------------------------------------------------------------------------
// Destructor
//
PlayState_Victory::~PlayState_Victory()
{
}


//-----------------------------------------------------------------------------------------------
// Checks for input to return to the main menu
//
void PlayState_Victory::ProcessInput()
{
	InputSystem* input = InputSystem::GetInstance();

	// Keyboard input
	bool spacePressed = input->WasKeyJustPressed(' ');

	if (spacePressed)
	{
		Game::GetInstance()->TransitionToGameState(new GameState_MainMenu());
	}
	else
	{
		// Check all controllers for press
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
// Enter update
//
bool PlayState_Victory::Enter()
{
	return m_transitionTimer.HasIntervalElapsed();
}


//-----------------------------------------------------------------------------------------------
// Update
//
void PlayState_Victory::Update()
{
	// Nothing!
}


//-----------------------------------------------------------------------------------------------
// Leave
//
bool PlayState_Victory::Leave()
{
	return m_transitionTimer.HasIntervalElapsed();
}


//-----------------------------------------------------------------------------------------------
// Renders the enter transition
//
void PlayState_Victory::Render_Enter() const
{
	Game::GetWorld()->Render();
	DebugRenderSystem::Draw2DText(Stringf("Victory Enter: %.2f seconds remaining", m_transitionTimer.GetTimeUntilIntervalEnds()), Window::GetInstance()->GetWindowBounds(), 0.f);
}


//-----------------------------------------------------------------------------------------------
// Renders the running state
//
void PlayState_Victory::Render() const
{
	Game::GetWorld()->Render();
	DebugRenderSystem::Draw2DText(Stringf("Victory!: Press Start/Space/A to return"), Window::GetInstance()->GetWindowBounds(), 0.f);
}


//-----------------------------------------------------------------------------------------------
// Renders the leave transition
//
void PlayState_Victory::Render_Leave() const
{
	Game::GetWorld()->Render();
	DebugRenderSystem::Draw2DText(Stringf("Victory Leave: %.2f seconds remaining", m_transitionTimer.GetTimeUntilIntervalEnds()), Window::GetInstance()->GetWindowBounds(), 0.f);
}
