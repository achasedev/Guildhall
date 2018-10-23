#include "Game/Framework/Game.hpp"
#include "Game/Framework/World.hpp"
#include "Game/Framework/GameCamera.hpp"
#include "Game/Framework/WaveManager.hpp"
#include "Game/Framework/PlayStates/PlayState_Wave.hpp"
#include "Game/GameStates/GameState_Playing.hpp"
#include "Game/Framework/PlayStates/PlayState_Rest.hpp"
#include "Game/Framework/PlayStates/PlayState_Victory.hpp"
#include "Game/Entity/Player.hpp"

#include "Engine/Core/Window.hpp"
#include "Engine/Core/Utility/StringUtils.hpp"
#include "Engine/Rendering/DebugRendering/DebugRenderSystem.hpp"

PlayState_Wave::PlayState_Wave()
	: PlayState(WAVE_TRANSITION_IN_TIME, WAVE_TRANSITION_OUT_TIME)
{
}

PlayState_Wave::~PlayState_Wave()
{
}

void PlayState_Wave::ProcessInput()
{
	// Check player input
	Player** players = Game::GetPlayers();

	for (int i = 0; i < MAX_PLAYERS; ++i)
	{
		if (players[i] != nullptr)
		{
			players[i]->ProcessGameplayInput();
		}
	}
}

void PlayState_Wave::Update()
{
	WaveManager* waveMan = Game::GetWaveManager();

	// Update the wave logic first
	waveMan->Update();

	UpdateWorldAndCamera();

	// Check for end of wave
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
}


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


void PlayState_Wave::Render_Enter() const
{
	Game::GetWorld()->Render();

	DebugRenderSystem::Draw2DText(Stringf("Wave Enter: %.2f seconds remaining", m_transitionTimer.GetTimeUntilIntervalEnds()), Window::GetInstance()->GetWindowBounds(), 0.f);
}

void PlayState_Wave::Render() const
{
	Game::GetWorld()->Render();

	DebugRenderSystem::Draw2DText(Stringf("Wave %i", Game::GetWaveManager()->GetCurrentWaveNumber()), Window::GetInstance()->GetWindowBounds(), 0.f);
}

void PlayState_Wave::Render_Leave() const
{
	Game::GetWorld()->Render();

	DebugRenderSystem::Draw2DText(Stringf("Wave Leave: %.2f seconds remaining", m_transitionTimer.GetTimeUntilIntervalEnds()), Window::GetInstance()->GetWindowBounds(), 0.f);
}
