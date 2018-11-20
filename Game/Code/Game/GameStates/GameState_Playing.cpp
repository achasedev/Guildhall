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
#include "Game/Framework/VoxelGrid.hpp"
#include "Game/Framework/VoxelFont.hpp"
#include "Game/Framework/GameCommon.hpp"
#include "Game/Framework/GameCamera.hpp"
#include "Game/Framework/CampaignManager.hpp"
#include "Game/GameStates/GameState_Playing.hpp"
#include "Game/Framework/PlayStates/PlayState.hpp"
#include "Game/Framework/PlayStates/PlayState_Rest.hpp"
#include "Game/Framework/PlayStates/PlayState_Stage.hpp"
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
	: GameState(0.f, 0.f)
{
}


//-----------------------------------------------------------------------------------------------
// Base destructor
//
GameState_Playing::~GameState_Playing()
{
	if (m_currentState != nullptr)
	{
		delete m_currentState;
		m_currentState = nullptr;
	}

	if (m_transitionState != nullptr)
	{
		delete m_transitionState;
		m_transitionState = nullptr;
	}
}


//-----------------------------------------------------------------------------------------------
// Sets up state before updating
//
bool GameState_Playing::Enter()
{
 	// Set up the mouse for FPS controls
	Mouse& mouse = InputSystem::GetMouse();

	mouse.ShowMouseCursor(false);
	mouse.LockCursorToClient(true);
	mouse.SetCursorMode(CURSORMODE_RELATIVE);

	// Create the players
	Player** players = Game::GetPlayers();

	for (int i = 0; i < MAX_PLAYERS; ++i)
	{
		if (InputSystem::GetInstance()->GetController(i).IsConnected())
		{
			players[i] = new Player(i);
			players[i]->SetPosition(Vector3(50.f * (float)i + 30.f, 0.f, 60.f));
		}
	}

	Game::GetCampaignManager()->Initialize("Data/Spawning.xml");
	Game::GetWorld()->InititalizeForStage(Game::GetCampaignManager()->GetNextStage());

	TransitionToPlayState(new PlayState_Stage());

	return true;
}


//-----------------------------------------------------------------------------------------------
// Cleans up/finishes any operations before the game leaves this state
//
bool GameState_Playing::Leave()
{
	Game::GetCampaignManager()->CleanUp();
	Game::GetWorld()->CleanUp();

	// Delete the players
	Player** players = Game::GetPlayers();
	for (int i = 0; i < MAX_PLAYERS; ++i)
	{
		if (players[i] != nullptr)
		{
			delete players[i];
			players[i] = nullptr;
		}
	}

	return true;
}


//-----------------------------------------------------------------------------------------------
// Renders the enter state transition
//
void GameState_Playing::Render_Enter() const
{
	Render();
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
// Returns true if all current players are dead
//
bool GameState_Playing::AreAllPlayersDead() const
{
	bool allDead = true;

	for (int i = 0; i < MAX_PLAYERS; ++i)
	{
		if (Game::IsPlayerAlive(i))
		{
			allDead = false;
			break;
		}
	}

	return allDead;
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
				m_currentState->StartUpdating();
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
	else if (m_currentState != nullptr)
	{
		m_currentState->Render();
	}

	// Render the HUD
	Game::DrawInGameUI();
}


//-----------------------------------------------------------------------------------------------
// Renders the leave state transition
//
void GameState_Playing::Render_Leave() const
{
	Render();
}
