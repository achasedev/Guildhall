/************************************************************************/
/* File: GameState_Playing.cpp
/* Author: Andrew Chase
/* Date: May 21st, 2018
/* Description: Implementation of the GameState_Playing class
/************************************************************************/
#include "Game/Entity/Player.hpp"
#include "Game/Framework/App.hpp"
#include "Game/Framework/Game.hpp"
#include "Game/Environment/Map.hpp"
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
	delete m_player;
	m_player = nullptr;
}


//-----------------------------------------------------------------------------------------------
// Sets up state before updating
//
void GameState_Playing::Enter()
{
	// Make the player
	m_player = new Player();

	// Make the map
	m_map = new Map();
	m_map->Intialize(AABB2(Vector2(-100.f, -100.f), Vector2(100.f, 100.f)), 0.f, 20.f, IntVector2(8, 8), "Data/Images/Map.jpg");

	Game::GetRenderScene()->AddCamera(m_player->GetCamera());
	Game::GetRenderScene()->AddLight(Light::CreateDirectionalLight(Vector3::ZERO, Vector3(0.1f, -1.f, 0.f), Rgba(200, 200, 200, 160)));
	Game::GetRenderScene()->SetAmbience(Rgba(255, 255, 255, 50));
 
	// Test the debug render system
 	DebugRenderSystem::SetWorldCamera(m_player->GetCamera());
}


//-----------------------------------------------------------------------------------------------
// Cleans up/finishes any operations before the game leaves this state
//
void GameState_Playing::Leave()
{
	TODO("Clear the render scene");
}


//-----------------------------------------------------------------------------------------------
// Returns the current map of the play state
//
Map* GameState_Playing::GetMap() const
{
	return m_map;
}


//-----------------------------------------------------------------------------------------------
// Checks for input
//
void GameState_Playing::ProcessInput()
{
	m_player->ProcessInput();

	InputSystem* input = InputSystem::GetInstance();

	if (input->WasKeyJustPressed('I'))
	{
		Light* light = Light::CreatePointLight(m_player->transform.position, Rgba::WHITE, Vector3(0.f, 0.f, 0.001f));
		Game::GetRenderScene()->AddLight(light);
	}

	Mouse& mouse = InputSystem::GetMouse();

	if (mouse.WasButtonJustPressed(MOUSEBUTTON_LEFT) || mouse.WasButtonJustPressed(MOUSEBUTTON_RIGHT))
	{
		mouse.ShowMouseCursor(false);
		mouse.LockCursorToClient(true);
		mouse.SetCursorMode(CURSORMODE_RELATIVE);
	}
	
	if (mouse.WasButtonJustReleased(MOUSEBUTTON_LEFT) || mouse.WasButtonJustReleased(MOUSEBUTTON_RIGHT))
	{
		mouse.ShowMouseCursor(true);
		mouse.LockCursorToClient(false);
		mouse.SetCursorMode(CURSORMODE_ABSOLUTE);
	}
}


//-----------------------------------------------------------------------------------------------
// Updates the play state
//
void GameState_Playing::Update()
{
	m_player->Update(Game::GetDeltaTime());	
}


//-----------------------------------------------------------------------------------------------
// Renders the gameplay state to screen
//
void GameState_Playing::Render() const
{
	ForwardRenderingPath::Render(Game::GetRenderScene());
}
