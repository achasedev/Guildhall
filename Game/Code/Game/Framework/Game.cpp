/************************************************************************/
/* File: Game.cpp
/* Author: Andrew Chase
/* Date: March 24th, 2018
/* Description: Game class for general gameplay management
/************************************************************************/
#include "Game.hpp"
#include "Game/Entity/Tank.hpp"
//#include "Game/Entity/Bullet.hpp"
#include "Game/Entity/Player.hpp"
#include "Game/Environment/Map.hpp"
#include "Game/Framework/GameCommon.hpp"
#include "Game/GameStates/GameState.hpp"
#include "Game/GameStates/GameState_Loading.hpp"
#include "Game/GameStates/GameState_MainMenu.hpp"
#include "Game/GameStates/GameState_Playing.hpp"

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

// The singleton instance
Game* Game::s_instance = nullptr;

//-----------------------------------------------------------------------------------------------
// Default constructor, initialize any game members here (private)
//
Game::Game()
	: m_currentState(new GameState_Loading())
{
	m_gameClock = new Clock(Clock::GetMasterClock());
}


//-----------------------------------------------------------------------------------------------
// Destructor - clean up any allocated members (private)
//
Game::~Game()
{
	DeleteAllGameObjects();
	DeleteMap();
	DeletePlayer();

	delete m_renderScene;
	m_renderScene = nullptr;
}


//-----------------------------------------------------------------------------------------------
// Constructs the singleton game instance
//
void Game::Initialize()
{
	GUARANTEE_OR_DIE(s_instance == nullptr, "Error: Game::Initialize called when a Game instance already exists.");
	s_instance = new Game();

	// Set the game clock on the Renderer
	Renderer::GetInstance()->SetRendererGameClock(s_instance->m_gameClock);

	Matrix44 test = Matrix44::MakeRotation(Vector3(45.f, 45.f, 45.f));
	Vector4 rotation = test.Transform(Vector4(0.f, 0.f, 1.f, 0.f));

	s_instance->m_renderScene = new RenderScene("Game Scene");
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


//-----------------------------------------------------------------------------------------------
// Returns the game's render scene
//
RenderScene* Game::GetRenderScene()
{
	return s_instance->m_renderScene;
}


//-----------------------------------------------------------------------------------------------
// Returns the player character
//
Player* Game::GetPlayer()
{
	return s_instance->m_player;
}


//-----------------------------------------------------------------------------------------------
// Returns the list of GameObjects currently in the game
//
std::vector<GameObject*>& Game::GetGameObjects()
{
	return s_instance->m_gameObjects;
}


//-----------------------------------------------------------------------------------------------
// Spawns the player
//
void Game::InitializePlayer()
{
	s_instance->m_player = new Player();
}


//-----------------------------------------------------------------------------------------------
// Initializes the map using the map parameters provided
//
void Game::InitializeMap(const AABB2& worldBounds, float minHeight, float maxHeight, const IntVector2& chunkLayout, const std::string& fileName)
{
	s_instance->m_map = new Map();
	s_instance->m_map->Intialize(worldBounds, minHeight, maxHeight, chunkLayout, fileName);
}


//-----------------------------------------------------------------------------------------------
// Adds the GameObject to the list of GameObjects
//
void Game::AddGameObject(GameObject* object)
{
	s_instance->m_gameObjects.push_back(object);
}


//-----------------------------------------------------------------------------------------------
// Deletes the player character object
//
void Game::DeletePlayer()
{
	if (s_instance->m_player != nullptr)
	{
		delete s_instance->m_player;
		s_instance->m_player = nullptr;
	}
}


//-----------------------------------------------------------------------------------------------
// Deletes the map
//
void Game::DeleteMap()
{
	if (s_instance->m_map != nullptr)
	{
		delete s_instance->m_map;
		s_instance->m_map = nullptr;
	}
}


//-----------------------------------------------------------------------------------------------
// Deletes all GameObjects in the list and clears it
//
void Game::DeleteAllGameObjects()
{
	int numGameObjects = (int) s_instance->m_gameObjects.size();

	for (int objectIndex = 0; objectIndex < numGameObjects; ++objectIndex)
	{
		delete s_instance->m_gameObjects[objectIndex];
	}

	s_instance->m_gameObjects.clear();
}


//-----------------------------------------------------------------------------------------------
// Returns the map of the playstate, or nullptr otherwise
//
Map* Game::GetMap()
{
	return s_instance->m_map;
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
