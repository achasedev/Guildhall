/************************************************************************/
/* File: GameState_Playing.cpp
/* Author: Andrew Chase
/* Date: May 21st, 2018
/* Description: Implementation of the GameState_Playing class
/************************************************************************/
#include "Game/Framework/App.hpp"
#include "Game/Framework/Game.hpp"
#include "Game/Framework/GameCommon.hpp"
#include "Game/GameStates/GameState_Playing.hpp"

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
}


//-----------------------------------------------------------------------------------------------
// Sets up state before updating
//
void GameState_Playing::Enter()
{
 	// Set up the game camera
	Renderer* renderer = Renderer::GetInstance();
	m_gameCamera = new Camera();
	m_gameCamera->SetColorTarget(renderer->GetDefaultColorTarget());
	m_gameCamera->SetDepthTarget(renderer->GetDefaultDepthTarget());
	m_gameCamera->SetProjectionPerspective(45.f, 0.1f, 1000.f);
	m_gameCamera->LookAt(Vector3(0.f, 0.f, -5.0f), Vector3::ZERO);

	Game::GetRenderScene()->AddCamera(m_gameCamera);
	Game::GetRenderScene()->AddLight(Light::CreateDirectionalLight(Vector3::ZERO, Vector3::DIRECTION_DOWN, Rgba(255, 255, 255, 100)));
	Game::GetRenderScene()->SetAmbience(Rgba(255, 255, 255, 50));
 
 	// Set up the mouse for FPS controls
	Mouse& mouse = InputSystem::GetMouse();

	mouse.ShowMouseCursor(false);
	mouse.LockCursorToClient(true);
	mouse.SetCursorMode(CURSORMODE_RELATIVE);
 
 	DebugRenderSystem::SetWorldCamera(m_gameCamera);
}


//-----------------------------------------------------------------------------------------------
// Cleans up/finishes any operations before the game leaves this state
//
void GameState_Playing::Leave()
{
	TODO("Clear the render scene");
}


//-----------------------------------------------------------------------------------------------
// Checks for input
//
void GameState_Playing::ProcessInput()
{
}


//-----------------------------------------------------------------------------------------------
// Updates the play state
//
void GameState_Playing::Update()
{
}


//-----------------------------------------------------------------------------------------------
// Renders the gameplay state to screen
//
void GameState_Playing::Render() const
{
	ForwardRenderingPath::Render(Game::GetRenderScene());
}
