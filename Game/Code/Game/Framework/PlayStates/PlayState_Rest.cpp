/************************************************************************/
/* File: PlayState_Rest.cpp
/* Author: Andrew Chase
/* Date: October 24th 2018
/* Description: Implementation of the Rest PlayState
/************************************************************************/
#include "Game/Entity/Player.hpp"
#include "Game/Framework/Game.hpp"
#include "Game/Framework/World.hpp"
#include "Game/Framework/GameCamera.hpp"
#include "Game/Framework/CampaignManager.hpp"
#include "Game/GameStates/GameState_Playing.hpp"
#include "Game/Framework/PlayStates/PlayState_Rest.hpp"

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
		// Get the next stage and initialize the next world to enter
		CampaignStage* nextStage = Game::GetCampaignManager()->GetNextStage();
		m_worldToTransitionTo = new World();
		m_worldToTransitionTo->InititalizeForStage(nextStage);

		return true;
	}

	return false;
}


//-----------------------------------------------------------------------------------------------
// Update
//
void PlayState_Rest::Update()
{
	// Check if the players are near the move location
	bool playersReady = true;
	Vector2 moveLocation = Vector2(10.f, 128.f);

	Player** players = Game::GetPlayers();

	for (int i = 0; i < MAX_PLAYERS; ++i)
	{
		if (players[i] != nullptr)
		{
			Vector2 playerPos = players[i]->GetPosition().xz();

			float distance = (playerPos - moveLocation).GetLengthSquared();
			if (distance > 200.f)
			{
				playersReady = false;
				break;
			}
		}
	}

	if (playersReady)
	{
		m_gameState->TransitionToPlayState(new PlayState_Stage());
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
		Game::SetWorld(m_worldToTransitionTo);
		return true;
	}
	
	return false;
}


//-----------------------------------------------------------------------------------------------
// Renders the enter transition
//
void PlayState_Rest::Render_Enter() const
{
	Game::GetWorld()->DrawToGrid();
	DebugRenderSystem::Draw2DText(Stringf("Rest Enter: %.2f seconds remaining", m_transitionTimer.GetTimeUntilIntervalEnds()), Window::GetInstance()->GetWindowBounds(), 0.f);
}


//-----------------------------------------------------------------------------------------------
// Renders the state during normal updating
//
void PlayState_Rest::Render() const
{
	Game::GetWorld()->DrawToGrid();
	DebugRenderSystem::Draw2DText(Stringf("Rest - GOGOGO"), Window::GetInstance()->GetWindowBounds(), 0.f);
}


//-----------------------------------------------------------------------------------------------
// Renders the leave transition
//
void PlayState_Rest::Render_Leave() const
{
	// For testing, always enter from the west
	int xOffset = -1 * (int)(m_transitionTimer.GetElapsedTimeNormalized() * 256.f);

	Game::GetWorld()->DrawToGridWithOffset(IntVector3(xOffset, 0, 0));
	m_worldToTransitionTo->DrawToGridWithOffset(IntVector3(xOffset + 256, 0, 0));

	DebugRenderSystem::Draw2DText(Stringf("Rest Leave: %.2f seconds remaining", m_transitionTimer.GetTimeUntilIntervalEnds()), Window::GetInstance()->GetWindowBounds(), 0.f);
}
