/************************************************************************/
/* File: GameState_Playing.cpp
/* Author: Andrew Chase
/* Date: May 21st, 2018
/* Description: Implementation of the GameState_Playing class
/************************************************************************/
#include "Game/Entity/Player.hpp"
#include "Game/Framework/App.hpp"
#include "Game/Entity/NPCTank.hpp"
#include "Game/Framework/Game.hpp"
#include "Game/Environment/Map.hpp"
#include "Game/Entity/NPCSpawner.hpp"
#include "Game/Framework/GameCommon.hpp"
#include "Game/GameStates/GameState_Playing.hpp"

#include "Engine/Core/Window.hpp"
#include "Engine/Assets/AssetDB.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Rendering/Core/Camera.hpp"
#include "Engine/Rendering/Resources/Sampler.hpp"
#include "Engine/Rendering/Materials/Material.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Rendering/Core/Renderer.hpp"
#include "Engine/Rendering/Meshes/MeshGroup.hpp"
#include "Engine/Rendering/Core/Renderable.hpp"
#include "Engine/Rendering/Core/RenderScene.hpp"
#include "Engine/Rendering/Particles/ParticleEmitter.hpp"
#include "Engine/Rendering/DebugRendering/DebugRenderSystem.hpp"
#include "Engine/Rendering/Core/ForwardRenderingPath.hpp"

#include "Engine/Core/Time/ScopedProfiler.hpp"


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
	Game::DeleteAllGameObjects();
	Game::DeleteMap();
	Game::DeletePlayer();
}


//-----------------------------------------------------------------------------------------------
// Sets up state before updating
//
void GameState_Playing::Enter()
{
	// Make the player
	Game::InitializePlayer();

	// Make the map
	Game::InitializeMap(AABB2(Vector2(-100.f, -100.f), Vector2(100.f, 100.f)), 0.f, 50.f, IntVector2(8, 8), "Data/Images/Map.jpg");

	Camera* playerCamera = Game::GetPlayer()->GetCamera();
	Game::GetRenderScene()->AddCamera(playerCamera);
	Game::GetRenderScene()->AddLight(Light::CreateDirectionalLight(Vector3::ZERO, Vector3(0.1f, -1.f, 0.f), Rgba(200, 200, 200, 160)));
	Game::GetRenderScene()->SetAmbience(Rgba(255, 255, 255, 50));
 
	// Test the debug render system
 	DebugRenderSystem::SetWorldCamera(playerCamera);

	Mouse& mouse = InputSystem::GetMouse();

	mouse.SetCursorMode(CURSORMODE_RELATIVE);
	mouse.ShowMouseCursor(false);
	mouse.LockCursorToClient(true);

	// Testing
	NPCSpawner* spawner = new NPCSpawner(Vector3(10.f, 15.f, 10.f), 1);
	Game::AddGameObject(spawner);
}


//-----------------------------------------------------------------------------------------------
// Cleans up/finishes any operations before the game leaves this state
//
void GameState_Playing::Leave()
{
}


//-----------------------------------------------------------------------------------------------
// Checks for input
//
void GameState_Playing::ProcessInput()
{
	Player* player = Game::GetPlayer();
	player->ProcessInput();

	InputSystem* input = InputSystem::GetInstance();

	if (input->WasKeyJustPressed('I'))
	{
		Light* light = Light::CreatePointLight(player->transform.position, Rgba::WHITE, Vector3(0.f, 0.f, 0.001f));
		Game::GetRenderScene()->AddLight(light);
	}
}


//-----------------------------------------------------------------------------------------------
// Updates the play state
//
void GameState_Playing::Update()
{
	float deltaTime = Game::GetDeltaTime();

	Game::GetPlayer()->Update(deltaTime);
	
	std::vector<GameObject*>& gameObjects = Game::GetGameObjects();
	int numObjects = (int) gameObjects.size();

	for (int objectIndex = 0; objectIndex < numObjects; ++objectIndex)
	{
		gameObjects[objectIndex]->Update(deltaTime);
	}

	for (int objectIndex = numObjects - 1; objectIndex >= 0; --objectIndex)
	{
		if (gameObjects[objectIndex]->IsMarkedForDelete())
		{
			delete gameObjects[objectIndex];
			gameObjects.erase(gameObjects.begin() + objectIndex);
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Renders the gameplay state to screen
//
void GameState_Playing::Render() const
{
	ForwardRenderingPath::Render(Game::GetRenderScene());
}
