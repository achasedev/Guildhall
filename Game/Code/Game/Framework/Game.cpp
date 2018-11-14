/************************************************************************/
/* File: Game.cpp
/* Author: Andrew Chase
/* Date: March 24th, 2018
/* Description: Game class for general gameplay management
/************************************************************************/
#include "Game/Entity/Player.hpp"
#include "Game/Framework/Game.hpp"
#include "Game/Framework/World.hpp"
#include "Game/Framework/VoxelGrid.hpp"
#include "Game/Framework/GameCommon.hpp"
#include "Game/Framework/GameCamera.hpp"
#include "Game/Framework/CampaignManager.hpp"
#include "Game/GameStates/GameState_Loading.hpp"

#include "Engine/Core/Time/Clock.hpp"
#include "Engine/Rendering/Core/Camera.hpp"
#include "Engine/Rendering/Core/Renderer.hpp"
#include "Engine/Rendering/Core/RenderScene.hpp"
#include "Engine/Rendering/DebugRendering/DebugRenderSystem.hpp"
#include "Engine/Core/DeveloperConsole/Command.hpp"
#include "Engine/Core/Utility/XmlUtilities.hpp"


void Command_KillAll(Command& cmd)
{
	UNUSED(cmd);

	Game::GetWorld()->ParticalizeAllEntities();
}

// The singleton instance
Game* Game::s_instance = nullptr;

//-----------------------------------------------------------------------------------------------
// Default constructor, initialize any game members here (private)
//
Game::Game()
	: m_currentState(new GameState_Loading())
{
	// Clock
	m_gameClock = new Clock(Clock::GetMasterClock());

	// Camera
	Renderer* renderer = Renderer::GetInstance();
	m_gameCamera = new GameCamera();
	m_gameCamera->SetColorTarget(renderer->GetDefaultColorTarget());
	m_gameCamera->SetDepthTarget(renderer->GetDefaultDepthTarget());
	m_gameCamera->SetProjectionPerspective(45.f, 0.1f, 1000.f);
	m_gameCamera->LookAt(Vector3(128.f, 200.f, -50.0f), Vector3(0.f, 200.f, 0.f));

	DebugRenderSystem::SetWorldCamera(m_gameCamera);

	// Game world
	m_world = new World();

	// Voxel Grid
	m_voxelGrid = new VoxelGrid();
	m_voxelGrid->Initialize(IntVector3(256, 64, 256));

	for (int i = 0; i < MAX_PLAYERS; ++i)
	{
		m_players[i] = nullptr;
	}
}


//-----------------------------------------------------------------------------------------------
// Destructor - clean up any allocated members (private)
//
Game::~Game()
{
	if (m_world != nullptr)
	{
		delete m_world;
		m_world = nullptr;
	}

	if (m_voxelGrid != nullptr)
	{
		delete m_voxelGrid;
		m_voxelGrid = nullptr;
	}
}


//-----------------------------------------------------------------------------------------------
// Constructs the singleton game instance
//
void Game::Initialize()
{
	GUARANTEE_OR_DIE(s_instance == nullptr, "Error: Game::Initialize called when a Game instance already exists.");
	s_instance = new Game();

	// Campaign manager initialize here, since it refers to the instance above
	s_instance->m_campaignManager = new CampaignManager();

	// Set the game clock on the Renderer
	Renderer::GetInstance()->SetRendererGameClock(s_instance->m_gameClock);

	Command::Register("killall", "Kills all entities", Command_KillAll);
}


//-----------------------------------------------------------------------------------------------
// Deletes the singleton instance
//
void Game::ShutDown()
{
	delete s_instance;
	s_instance = nullptr;
}


//-----------------------------------------------------------------------------------------------
// Checks for input received this frame and updates states accordingly
//
void Game::ProcessInput()
{
	if (m_currentState != nullptr)
	{
		m_currentState->ProcessInput();
	}
}


//-----------------------------------------------------------------------------------------------
// Update the movement variables of every entity in the world, as well as update the game state
// based on the input this frame
//
void Game::Update()
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
// Draws to screen
//
void Game::Render() const
{
	m_voxelGrid->Clear();

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

	m_voxelGrid->BuildMeshAndDraw();
}


//-----------------------------------------------------------------------------------------------
// Returns the game state of the Game instance
//
GameState* Game::GetGameState() const
{
	return m_currentState;
}


//-----------------------------------------------------------------------------------------------
// Sets the pending state flag to the one given, so the next frame the game will switch to the
// given state
//
void Game::TransitionToGameState(GameState* newState)
{
	s_instance->m_transitionState = newState;
	s_instance->m_isTransitioning = true;

	if (s_instance->m_currentState != nullptr)
	{
		s_instance->m_currentState->StartLeaveTimer();
	}
}


//-----------------------------------------------------------------------------------------------
// Returns the voxel grid used by the game
//
VoxelGrid* Game::GetVoxelGrid()
{
	return s_instance->m_voxelGrid;
}


//-----------------------------------------------------------------------------------------------
// Returns the game clock
//
Clock* Game::GetGameClock()
{
	return s_instance->m_gameClock;
}


//-----------------------------------------------------------------------------------------------
// Returns the camera used to render game elements
//
GameCamera* Game::GetGameCamera()
{
	return s_instance->m_gameCamera;
}


//-----------------------------------------------------------------------------------------------
// Returns the frame time for the game clock
//
float Game::GetDeltaTime()
{
	return s_instance->m_gameClock->GetDeltaTime();
}


//-----------------------------------------------------------------------------------------------
// Returns the world used for all entities and terrain
//
World* Game::GetWorld()
{
	return s_instance->m_world;
}


//-----------------------------------------------------------------------------------------------
// Returns the players array (not necessarily full)
//
Player** Game::GetPlayers()
{
	return &s_instance->m_players[0];
}


//-----------------------------------------------------------------------------------------------
// Returns the Campaign Manager for the game
//
CampaignManager* Game::GetCampaignManager()
{
	return s_instance->m_campaignManager;
}


//-----------------------------------------------------------------------------------------------
// Returns whether the player at the given index exists and is not marked for delete (dead)
//
bool Game::IsPlayerAlive(unsigned int index)
{
	if (index >= MAX_PLAYERS)
	{
		return false;
	}

	Player* player = s_instance->m_players[index];
	return (player != nullptr && !player->IsMarkedForDelete());
}


//-----------------------------------------------------------------------------------------------
// Returns the singleton Game instance
//
Game* Game::GetInstance()
{
	return s_instance;
}
