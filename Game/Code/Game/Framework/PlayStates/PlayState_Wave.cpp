/************************************************************************/
/* File: PlayState_Wave.cpp
/* Author: Andrew Chase
/* Date: October 25th 2018
/* Description: Implementation of the Wave PlayState
/************************************************************************/
#include "Game/Framework/Game.hpp"
#include "Game/Framework/World.hpp"
#include "Game/Framework/GameCamera.hpp"
#include "Game/Framework/WaveManager.hpp"
#include "Game/Framework/PlayStates/PlayState_Wave.hpp"
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
PlayState_Wave::PlayState_Wave()
	: PlayState(WAVE_TRANSITION_IN_TIME, WAVE_TRANSITION_OUT_TIME)
{
}


//-----------------------------------------------------------------------------------------------
// Destructor
//
PlayState_Wave::~PlayState_Wave()
{
}


//-----------------------------------------------------------------------------------------------
// Checks for player gameplay input and applies it
//
void PlayState_Wave::ProcessInput()
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
// Updates the wave manager and then world
//
void PlayState_Wave::Update()
{
	WaveManager* waveMan = Game::GetWaveManager();

	// Update the wave logic first
	waveMan->Update();

	UpdateWorldAndCamera();

	// Check for end of wave and victory
	if (waveMan->IsCurrentWaveFinished())
	{
		if (waveMan->IsCurrentWaveFinal())
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
bool PlayState_Wave::Enter()
{
	UpdateWorldAndCamera();

	// Do stuff
	if (m_transitionTimer.HasIntervalElapsed())
	{
		Game::GetWaveManager()->StartNextWave();
		return true;
	}

	return false;
}


//-----------------------------------------------------------------------------------------------
// Updates the leave transition
//
bool PlayState_Wave::Leave()
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
void PlayState_Wave::Render_Enter() const
{
	Game::GetWorld()->Render();
	DebugRenderSystem::Draw2DText(Stringf("Wave Enter: %.2f seconds remaining", m_transitionTimer.GetTimeUntilIntervalEnds()), Window::GetInstance()->GetWindowBounds(), 0.f);
}


//-----------------------------------------------------------------------------------------------
// Renders the normal state of the game
//
void PlayState_Wave::Render() const
{
	Game::GetWorld()->Render();
	DebugRenderSystem::Draw2DText(Stringf("Wave %i of %i", Game::GetWaveManager()->GetCurrentWaveNumber() + 1, Game::GetWaveManager()->GetWaveCount()), Window::GetInstance()->GetWindowBounds(), 0.f);
}


//-----------------------------------------------------------------------------------------------
// Renders the leave transition of the game
//
void PlayState_Wave::Render_Leave() const
{
	Game::GetWorld()->Render();
	DebugRenderSystem::Draw2DText(Stringf("Wave Leave: %.2f seconds remaining", m_transitionTimer.GetTimeUntilIntervalEnds()), Window::GetInstance()->GetWindowBounds(), 0.f);
}
