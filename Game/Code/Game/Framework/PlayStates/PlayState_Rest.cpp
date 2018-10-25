/************************************************************************/
/* File: PlayState_Rest.cpp
/* Author: Andrew Chase
/* Date: October 24th 2018
/* Description: Implementation of the Rest PlayState
/************************************************************************/
#include "Game/Framework/PlayStates/PlayState_Rest.hpp"
#include "Game/Framework/Game.hpp"
#include "Game/Framework/World.hpp"
#include "Game/Framework/GameCamera.hpp"
#include "Game/GameStates/GameState_Playing.hpp"
#include "Game/Entity/Player.hpp"

// For debug rendering
#include "Engine/Core/Window.hpp"
#include "Engine/Core/Utility/StringUtils.hpp"
#include "Engine/Rendering/DebugRendering/DebugRenderSystem.hpp"


//-----------------------------------------------------------------------------------------------
// Constructor
//
PlayState_Rest::PlayState_Rest()
	: PlayState(REST_TRANSITION_IN_TIME, REST_TRANSITION_OUT_TIME)
{
}


//-----------------------------------------------------------------------------------------------
// Destructor
//
PlayState_Rest::~PlayState_Rest()
{

}


//-----------------------------------------------------------------------------------------------
// Checks for gameplay input on the players
//
void PlayState_Rest::ProcessInput()
{
	// Check player input
	Player** players = Game::GetPlayers();

	for (int i = 0; i < MAX_PLAYERS; ++i)
	{
		if (Game::IsPlayerAlive(i))
		{
			players[i]->ProcessGameplayInput();
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Enter update step
//
bool PlayState_Rest::Enter()
{
	// Respawn the dead players
	Player** players = Game::GetPlayers();

	for (int i = 0; i < MAX_PLAYERS; ++i)
	{
		if (players[i] != nullptr && players[i]->IsMarkedForDelete())
		{
			players[i]->Respawn();
		}
	}

	UpdateWorldAndCamera();

	if (m_transitionTimer.HasIntervalElapsed())
	{
		m_restTimer.SetInterval(REST_INTERVAL);
		return true;
	}

	return false;
}


//-----------------------------------------------------------------------------------------------
// Update
//
void PlayState_Rest::Update()
{
	if (m_restTimer.HasIntervalElapsed())
	{
		m_gameState->TransitionToPlayState(new PlayState_Wave());
	}
	else
	{
		UpdateWorldAndCamera();

		// Camera
		GameCamera* camera = Game::GetGameCamera();
		if (!camera->IsEjected())
		{
			Game::GetGameCamera()->UpdatePositionBasedOnPlayers();
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Leave update
//
bool PlayState_Rest::Leave()
{
	UpdateWorldAndCamera();

	if (m_transitionTimer.HasIntervalElapsed())
	{
		return true;
	}

	return false;
}


//-----------------------------------------------------------------------------------------------
// Renders the enter transition
//
void PlayState_Rest::Render_Enter() const
{
	Game::GetWorld()->Render();

	DebugRenderSystem::Draw2DText(Stringf("Rest Enter: %.2f seconds remaining", m_transitionTimer.GetTimeUntilIntervalEnds()), Window::GetInstance()->GetWindowBounds(), 0.f);
}


//-----------------------------------------------------------------------------------------------
// Renders the state during normal updating
//
void PlayState_Rest::Render() const
{
	Game::GetWorld()->Render();
	DebugRenderSystem::Draw2DText(Stringf("Rest: %.2f seconds remaining", m_restTimer.GetTimeUntilIntervalEnds()), Window::GetInstance()->GetWindowBounds(), 0.f);
}


//-----------------------------------------------------------------------------------------------
// Renders the leave transition
//
void PlayState_Rest::Render_Leave() const
{
	Game::GetWorld()->Render();

	DebugRenderSystem::Draw2DText(Stringf("Rest Leave: %.2f seconds remaining", m_transitionTimer.GetTimeUntilIntervalEnds()), Window::GetInstance()->GetWindowBounds(), 0.f);
}
