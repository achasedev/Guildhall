/************************************************************************/
/* File: PlayState_Stage.cpp
/* Author: Andrew Chase
/* Date: October 25th 2018
/* Description: Implementation of the Stage PlayState
/************************************************************************/
#include "Game/Framework/Game.hpp"
#include "Game/Framework/World.hpp"
#include "Game/Framework/GameCamera.hpp"
#include "Game/Framework/CampaignManager.hpp"
#include "Game/Framework/PlayStates/PlayState_Stage.hpp"
#include "Game/GameStates/GameState_Playing.hpp"
#include "Game/Framework/PlayStates/PlayState_Rest.hpp"
#include "Game/Framework/PlayStates/PlayState_Victory.hpp"
#include "Game/Framework/PlayStates/PlayState_Defeat.hpp"
#include "Game/Entity/Player.hpp"

// For debug rendering the state
#include "Engine/Core/Window.hpp"
#include "Engine/Core/Utility/StringUtils.hpp"
#include "Engine/Rendering/DebugRendering/DebugRenderSystem.hpp"


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
// Updates the Stage manager and then world
//
void PlayState_Stage::Update()
{
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

	// Check for defeat
	if (m_gameState->AreAllPlayersDead())
	{
		m_gameState->TransitionToPlayState(new PlayState_Defeat());
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
	UpdateWorldAndCamera();

	// Do stuff
	if (m_transitionTimer.HasIntervalElapsed())
	{
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
	DebugRenderSystem::Draw2DText(Stringf("Stage Enter: %.2f seconds remaining", m_transitionTimer.GetTimeUntilIntervalEnds()), Window::GetInstance()->GetWindowBounds(), 0.f);
}


//-----------------------------------------------------------------------------------------------
// Renders the normal state of the game
//
void PlayState_Stage::Render() const
{
	Game::GetWorld()->DrawToGrid();
	DebugRenderSystem::Draw2DText(Stringf("Stage %i of %i", Game::GetCampaignManager()->GetCurrentStageNumber() + 1, Game::GetCampaignManager()->GetStageCount()), Window::GetInstance()->GetWindowBounds(), 0.f);
}


//-----------------------------------------------------------------------------------------------
// Renders the leave transition of the game
//
void PlayState_Stage::Render_Leave() const
{
	Game::GetWorld()->DrawToGrid();
	DebugRenderSystem::Draw2DText(Stringf("Stage Leave: %.2f seconds remaining", m_transitionTimer.GetTimeUntilIntervalEnds()), Window::GetInstance()->GetWindowBounds(), 0.f);
}
