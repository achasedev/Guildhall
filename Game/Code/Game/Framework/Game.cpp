/************************************************************************/
/* File: Game.cpp
/* Author: Andrew Chase
/* Date: March 24th, 2018
/* Description: Game class for general gameplay management
/************************************************************************/
#include "Game.hpp"
#include "Game/Entity/Tank.hpp"
#include "Game/Entity/Player.hpp"
#include "Game/Entity/Swarmer.hpp"
#include "Game/Environment/Map.hpp"
#include "Game/Framework/GameCommon.hpp"
#include "Game/GameStates/GameState.hpp"
#include "Game/GameStates/GameState_Loading.hpp"
#include "Game/GameStates/GameState_MainMenu.hpp"
#include "Game/GameStates/GameState_Playing.hpp"

#include "Engine/Core/Time/ProfileLogScoped.hpp"

#include "Engine/Assets/AssetDB.hpp"
#include "Engine/Core/GameObject.hpp"
#include "Engine/Core/Time/Clock.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Rendering/Core/Renderer.hpp"
#include "Engine/Core/Utility/StringUtils.hpp"
#include "Engine/Rendering/Core/Renderable.hpp"
#include "Engine/Rendering/Core/RenderScene.hpp"
#include "Engine/Core/DeveloperConsole/Command.hpp"
#include "Engine/Core/Utility/ErrorWarningAssert.hpp"
#include "Engine/Rendering/Materials/MaterialInstance.hpp"
#include "Engine/Rendering/DebugRendering/DebugRenderSystem.hpp"

// Console commands
void Command_KillAll(Command& cmd);

// The singleton instance
Game* Game::s_instance = nullptr;

//-----------------------------------------------------------------------------------------------
// Destructor - clean up any allocated members (private)
//
Game::~Game()
{
	delete m_map;
	m_map = nullptr;

	delete m_player;
	m_player = nullptr;

	delete m_gameClock;
	m_gameClock = nullptr;

	delete m_renderScene;
	m_renderScene = nullptr;

	DebugRenderSystem::SetWorldCamera(nullptr);
}


//-----------------------------------------------------------------------------------------------
// Constructs the singleton game instance
//
void Game::Initialize()
{
	GUARANTEE_OR_DIE(s_instance == nullptr, "Error: Game::Initialize called when a Game instance already exists.");
	s_instance = new Game();
	s_instance->SetupInitialState();

	// Set the game clock on the Renderer
	Renderer::GetInstance()->SetRendererGameClock(s_instance->m_gameClock);

	Swarmer::InitializeConsoleCommands();
	Command::Register("KillAll", "Kills all non-player entities on the map",		Command_KillAll);
}


void Game::SetupInitialState()
{
	m_currentState = new GameState_Loading();

	m_renderScene = new RenderScene("Game Scene");
	m_gameClock = new Clock(Clock::GetMasterClock());
	m_player = new Player();

	Camera* playerCamera = m_player->GetCamera();
	m_renderScene->AddCamera(playerCamera);

	// Lights
	Light* directionalLight = Light::CreateDirectionalLight(Vector3(10.f, 50.f, 10.f), Vector3(-1.f, -1.f, 0.f), Rgba(200, 200, 200, 255));
	directionalLight->SetShadowCasting(true);

	m_renderScene->AddLight(directionalLight);
	m_renderScene->SetAmbience(Rgba(255, 255, 255, 50));

	DebugRenderSystem::SetWorldCamera(playerCamera);
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
	m_currentState->ProcessInput();
}


//-----------------------------------------------------------------------------------------------
// Update the movement variables of every entity in the world, as well as update the game state
// based on the input this frame
//
void Game::Update()
{
	PROFILE_LOG_SCOPE("Game::Update");

	// Check for state change
	CheckToUpdateGameState();

	// Update the current state
	m_currentState->Update();
}


//-----------------------------------------------------------------------------------------------
// Draws to screen
//
void Game::Render() const
{
	m_currentState->Render();
}


//-----------------------------------------------------------------------------------------------
// Sets the pending state flag to the one given, so the next frame the game will switch to the
// given state
//
void Game::TransitionToGameState(GameState* newState)
{
	s_instance->m_pendingState = newState;
}


//-----------------------------------------------------------------------------------------------
// Returns the current GameState of the singleton Game instance
//
GameState* Game::GetCurrentGameState()
{
	return s_instance->m_currentState;
}


//-----------------------------------------------------------------------------------------------
// Returns the game clock
//
Clock* Game::GetGameClock()
{
	return s_instance->m_gameClock;
}


//-----------------------------------------------------------------------------------------------
// Returns the frame time for the game clock
//
float Game::GetDeltaTime()
{
	return s_instance->m_gameClock->GetDeltaTime();
}


Map* Game::GetMap()
{
	return s_instance->m_map;
}

Player* Game::GetPlayer()
{
	return s_instance->m_player;
}

RenderScene* Game::GetRenderScene()
{
	return s_instance->m_renderScene;
}

//-----------------------------------------------------------------------------------------------
// Returns the singleton Game instance
//
Game* Game::GetInstance()
{
	return s_instance;
}


//-----------------------------------------------------------------------------------------------
// Checks if there is a pending 
//
void Game::CheckToUpdateGameState()
{
	// Have a state pending
	if (m_pendingState != nullptr)
	{
		if (m_currentState != nullptr)
		{
			// Leave and destroy current
			m_currentState->Leave();
			delete m_currentState;
		}

		// Set new as current
		m_currentState = m_pendingState;
		m_pendingState = nullptr;

		// Enter the new state
		m_currentState->Enter();
	}
}

//////////////////////////////////////////////////////////////////////////
//--------------------------CONSOLE COMMANDS------------------------------
//////////////////////////////////////////////////////////////////////////

void Command_KillAll(Command& cmd)
{
	UNUSED(cmd);
	Map* map = Game::GetMap();

	if (map != nullptr)
	{
		map->KillAllEnemies();
		ConsolePrintf(Rgba::GREEN, "All enemies killed");
	}
	else
	{
		ConsoleWarningf("No map initialized.");
	}
}

