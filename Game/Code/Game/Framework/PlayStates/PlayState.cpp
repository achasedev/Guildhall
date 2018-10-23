#include "Game/Framework/Game.hpp"
#include "Game/GameStates/GameState_Playing.hpp"
#include "Game/Framework/PlayStates/PlayState.hpp"
#include "Game/Framework/World.hpp"
#include "Game/Framework/GameCamera.hpp"

PlayState::PlayState(float transitionInTime, float transitionOutTime)
	: m_transitionInTime(transitionInTime)
	, m_transitionOutTime(transitionOutTime)
	, m_transitionTimer(Game::GetGameClock())
	, m_gameState(dynamic_cast<GameState_Playing*>(Game::GetInstance()->GetGameState()))
{
}

PlayState::~PlayState()
{
}


void PlayState::StartEnterTimer()
{
	m_transitionTimer.SetInterval(m_transitionInTime);
}

void PlayState::StartLeaveTimer()
{
	m_transitionTimer.SetInterval(m_transitionOutTime);
}

void PlayState::UpdateWorldAndCamera()
{
	// Then update the world
	Game::GetWorld()->Update();

	// Camera
	GameCamera* camera = Game::GetGameCamera();
	if (!camera->IsEjected())
	{
		Game::GetGameCamera()->UpdatePositionBasedOnPlayers();
	}
}

