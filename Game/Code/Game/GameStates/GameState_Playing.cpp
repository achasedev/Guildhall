/************************************************************************/
/* File: GameState_Playing.cpp
/* Author: Andrew Chase
/* Date: May 21st, 2018
/* Description: Implementation of the GameState_Playing class
/************************************************************************/
#include "Game/Framework/Game.hpp"
#include "Game/Framework/World.hpp"
#include "Game/Framework/GameCommon.hpp"
#include "Game/GameStates/GameState_Playing.hpp"

#include "Engine/Input/InputSystem.hpp"
#include "Engine/Rendering/Core/Camera.hpp"
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
 	// Set up the game camera
	Game* game = Game::GetInstance();
 
 	// Set up the mouse for FPS controls
	Mouse& mouse = InputSystem::GetMouse();

	mouse.ShowMouseCursor(false);
	mouse.LockCursorToClient(true);
	mouse.SetCursorMode(CURSORMODE_RELATIVE);
 
 	DebugRenderSystem::SetWorldCamera(game->m_gameCamera);
	DebugRenderSystem::DrawUVSphere(Vector3::ZERO, 10000.f);
}


//-----------------------------------------------------------------------------------------------
// Cleans up/finishes any operations before the game leaves this state
//
void GameState_Playing::Leave()
{
}


//-----------------------------------------------------------------------------------------------
// Updates the camera transform based on keyboard and mouse input
//
void GameState_Playing::UpdateCameraOnInput()
{
	float deltaTime = Game::GetDeltaTime();
	InputSystem* input = InputSystem::GetInstance();

	// Translating the camera
	Vector3 translationOffset = Vector3::ZERO;
	if (input->IsKeyPressed('W'))								{ translationOffset.z += 1.f; }		// Forward
	if (input->IsKeyPressed('S'))								{ translationOffset.z -= 1.f; }		// Left
	if (input->IsKeyPressed('A'))								{ translationOffset.x -= 1.f; }		// Back
	if (input->IsKeyPressed('D'))								{ translationOffset.x += 1.f; }		// Right
	if (input->IsKeyPressed(InputSystem::KEYBOARD_SPACEBAR))	{ translationOffset.y += 1.f; }		// Up
	if (input->IsKeyPressed('X'))								{ translationOffset.y -= 1.f; }		// Down

	if (input->IsKeyPressed(InputSystem::KEYBOARD_SHIFT))
	{
		translationOffset *= 50.f;
	}

	translationOffset *= CAMERA_TRANSLATION_SPEED * deltaTime;

	Camera* gameCamera = Game::GetGameCamera();
	gameCamera->TranslateLocal(translationOffset);

	// Rotating the camera
	Mouse& mouse = InputSystem::GetMouse();
	IntVector2 mouseDelta = mouse.GetMouseDelta();

	Vector2 rotationOffset = Vector2((float) mouseDelta.y, (float) mouseDelta.x) * 0.12f;
	Vector3 rotation = Vector3(rotationOffset.x * CAMERA_ROTATION_SPEED * deltaTime, rotationOffset.y * CAMERA_ROTATION_SPEED * deltaTime, 0.f);

	gameCamera->Rotate(rotation);	
}


//-----------------------------------------------------------------------------------------------
// Checks for input
//
void GameState_Playing::ProcessInput()
{
	UpdateCameraOnInput();
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
}
