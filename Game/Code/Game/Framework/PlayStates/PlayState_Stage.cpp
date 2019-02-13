/************************************************************************/
/* File: PlayState_Stage.cpp
/* Author: Andrew Chase
/* Date: October 25th 2018
/* Description: Implementation of the Stage PlayState
/************************************************************************/
#include "Game/Entity/Player.hpp"
#include "Game/Framework/Game.hpp"
#include "Game/Framework/World.hpp"
#include "Game/Framework/GameCamera.hpp"
#include "Game/Framework/CampaignManager.hpp"
#include "Game/Framework/PlayStates/PlayState_Stage.hpp"
#include "Game/GameStates/GameState_Playing.hpp"
#include "Game/Framework/PlayStates/PlayState_Rest.hpp"
#include "Game/Framework/PlayStates/PlayState_Victory.hpp"
#include "Game/Framework/PlayStates/PlayState_Pause.hpp"
#include "Game/Framework/PlayStates/PlayState_Defeat.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Time/Clock.hpp"

//-----------------------------------------------------------------------------------------------
// Constructor
//
PlayState_Stage::PlayState_Stage()
	: PlayState(STAGE_TRANSITION_IN_TIME, STAGE_TRANSITION_OUT_TIME)
{
}


//-----------------------------------------------------------------------------------------------
// Destructor
//
PlayState_Stage::~PlayState_Stage()
{
}


//-----------------------------------------------------------------------------------------------
// Checks for player gameplay input and applies it
//
void PlayState_Stage::ProcessInput()
{
	int pauseIndex = CheckForPause();

	if (pauseIndex != -1)
	{
		m_gameState->PushOverrideState(new PlayState_Pause(Game::GetPlayers()[pauseIndex]));
	}
	else
	{
		// Check player gameplay input
		Player** players = Game::GetPlayers();

		for (int i = 0; i < MAX_PLAYERS; ++i)
		{
			if (players[i] != nullptr && !players[i]->IsRespawning())
			{
				players[i]->ProcessGameplayInput();
			}
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Updates the Stage manager and then world
//
void PlayState_Stage::Update()
{
	// Check the controllers
	m_gameState->PerformControllerCheck();

	CampaignManager* stageMan = Game::GetCampaignManager();

	// Update the stage logic first
	stageMan->Update();

	UpdateWorldAndCamera();

	// Check for end of stage and victory
	if (stageMan->IsCurrentStageFinished())
	{
		if (stageMan->IsCurrentStageFinal())
		{
			m_gameState->TransitionToPlayState(new PlayState_Victory());
		}
		else
		{
			m_gameState->TransitionToPlayState(new PlayState_Rest());
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Update the enter transition
//
bool PlayState_Stage::Enter()
{	
	UpdateWorldAndCamera();

	// Do stuff
	if (m_transitionTimer.HasIntervalElapsed())
	{
		Game::GetCampaignManager()->StartNextStage();
		return true;
	}

	return false;
}


//-----------------------------------------------------------------------------------------------
// Updates the leave transition
//
bool PlayState_Stage::Leave()
{
	// Cool effect - set the clock time scale
	float timeScale = 1.0f;

	if (m_transitionTimer.GetElapsedTimeNormalized() < 0.5f)
	{
		timeScale = ClampFloatZeroToOne(1.1f - GetFractionInRange(m_transitionTimer.GetElapsedTime(), 0.f, 0.5f * STAGE_TRANSITION_OUT_TIME));
	}
	else
	{
		timeScale = ClampFloatZeroToOne(GetFractionInRange(m_transitionTimer.GetElapsedTime(), 0.5f * STAGE_TRANSITION_OUT_TIME, STAGE_TRANSITION_OUT_TIME) + 0.1f);
	}

	Game::GetGameClock()->SetScale(timeScale);

	UpdateWorldAndCamera();

	// Do stuff
	if (m_transitionTimer.HasIntervalElapsed())
	{
		Game::GetGameClock()->SetScale(1.0f);
		return true;
	}

	return false;
}


//-----------------------------------------------------------------------------------------------
// Renders the enter transition
//
void PlayState_Stage::Render_Enter() const
{
	Game::GetWorld()->DrawToGrid();
	Game::DrawPlayerHUD();
}


//-----------------------------------------------------------------------------------------------
// Renders the normal state of the game
//
void PlayState_Stage::Render() const
{
	Game::GetWorld()->DrawToGrid();
	Game::DrawPlayerHUD();
}


//-----------------------------------------------------------------------------------------------
// Renders the leave transition of the game
//
void PlayState_Stage::Render_Leave() const
{
	Game::GetWorld()->DrawToGrid();
	Game::DrawPlayerHUD();
}
