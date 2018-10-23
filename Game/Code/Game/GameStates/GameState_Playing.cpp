/************************************************************************/
/* File: GameState_Playing.cpp
/* Author: Andrew Chase
/* Date: May 21st, 2018
/* Description: Implementation of the GameState_Playing class
/************************************************************************/
#include "Game/Entity/Player.hpp"
#include "Game/Entity/AIEntity.hpp"
#include "Game/Framework/Game.hpp"
#include "Game/Framework/World.hpp"
#include "Game/Framework/GameCommon.hpp"
#include "Game/Framework/GameCamera.hpp"
#include "Game/Framework/WaveManager.hpp"
#include "Game/GameStates/GameState_Playing.hpp"
#include "Game/Framework/PlayStates/PlayState.hpp"
#include "Game/Framework/PlayStates/PlayState_Wave.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Rendering/Core/Renderer.hpp"
#include "Engine/Rendering/Core/RenderScene.hpp"
#include "Engine/Rendering/Core/ForwardRenderingPath.hpp"
#include "Engine/Rendering/DebugRendering/DebugRenderSystem.hpp"


//-----------------------------------------------------------------------------------------------
// Base constructor
//
GameState_Playing::GameState_Playing()
{
}


//-----------------------------------------------------------------------------------------------
// Base destructor
//
GameState_Playing::~GameState_Playing()
{
}


//-----------------------------------------------------------------------------------------------
// Sets up state before updating
//
void GameState_Playing::Enter()
{
 	// Set up the mouse for FPS controls
	Mouse& mouse = InputSystem::GetMouse();

	mouse.ShowMouseCursor(false);
	mouse.LockCursorToClient(true);
	mouse.SetCursorMode(CURSORMODE_RELATIVE);

	Player** players = Game::GetPlayers();

	for (int i = 0; i < 1; ++i)
	{
		players[i] = new Player(i);
		players[i]->SetPosition(Vector3(50.f * (float) i + 30.f, 0.f, 60.f));
		players[i]->SetTeam(ENTITY_TEAM_PLAYER);

		Game::GetWorld()->AddEntity(players[i]);
	}

	Game::GetWorld()->Inititalize();
	Game::GetWaveManager()->Initialize("Data/Spawning.xml");

	TransitionToPlayState(new PlayState_Wave());
}


//-----------------------------------------------------------------------------------------------
// Cleans up/finishes any operations before the game leaves this state
//
void GameState_Playing::Leave()
{
}


//-----------------------------------------------------------------------------------------------
// Begins a transition to the given state from the current state
//
void GameState_Playing::TransitionToPlayState(PlayState* state)
{
	m_transitionState = state;
	m_isTransitioning = true;

	if (m_currentState != nullptr)
	{
		m_currentState->StartLeaveTimer();
	}
}


//-----------------------------------------------------------------------------------------------
// Checks for input
//
void GameState_Playing::ProcessInput()
{
	if (m_currentState != nullptr)
	{
		m_currentState->ProcessInput();
	}
	else
	{
		m_transitionState->ProcessInput();
	}

	if (InputSystem::GetInstance()->WasKeyJustPressed('L'))
	{
		Game::GetWorld()->ParticalizeAllEntities();
	}

	// Camera
	GameCamera* camera = Game::GetGameCamera();
	if (InputSystem::GetInstance()->WasKeyJustPressed('B'))
	{
		camera->ToggleEjected();
	}

	if (camera->IsEjected())
	{
		camera->UpdatePositionOnInput();
	}
}


//-----------------------------------------------------------------------------------------------
// Updates the play state
//
void GameState_Playing::Update()
{
	if (m_isTransitioning)
	{
		// Update on leave of the current state
		if (m_currentState != nullptr)
		{
			bool leaveFinished = m_currentState->Leave();

			if (leaveFinished)
			{
				delete m_currentState;
				m_currentState = nullptr;

				m_transitionState->StartEnterTimer();
			}
		}
		else // Update on enter of the transition state
		{
			bool enterFinished = m_transitionState->Enter();

			if (enterFinished)
			{
				m_currentState = m_transitionState;
				m_transitionState = nullptr;
				m_isTransitioning = false;
			}
		}
	}
	else
	{
		m_currentState->Update();
	}
}


//-----------------------------------------------------------------------------------------------
// Renders the gameplay state to screen
//
void GameState_Playing::Render() const
{
	if (m_isTransitioning)
	{
		if (m_currentState != nullptr)
		{
			m_currentState->Render_Leave();
		}
		else
		{
			m_transitionState->Render_Enter();
		}
	}
	else
	{
		m_currentState->Render();
	}
}
