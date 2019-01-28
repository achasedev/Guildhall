/************************************************************************/
/* File: GameState_Playing.cpp
/* Author: Andrew Chase
/* Date: May 21st, 2018
/* Description: Implementation of the GameState_Playing class
/************************************************************************/
#include "Game/Framework/Game.hpp"
#include "Game/Framework/GameCommon.hpp"
#include "Game/GameStates/GameState_Playing.hpp"
#include "Game/GameStates/GameState_MainMenu.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Rendering/Core/Camera.hpp"
#include "Engine/Rendering/Core/Renderer.hpp"
#include "Engine/Rendering/Core/RenderScene.hpp"
#include "Engine/Core/DeveloperConsole/DevConsole.hpp"
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
}


//-----------------------------------------------------------------------------------------------
// Sets up state before updating
//
bool GameState_Playing::Enter()
{
	return true;
}


//-----------------------------------------------------------------------------------------------
// Cleans up/finishes any operations before the game leaves this state
//
bool GameState_Playing::Leave()
{
	return m_transitionTimer.HasIntervalElapsed();
}


//-----------------------------------------------------------------------------------------------
// Renders the enter state transition
//
void GameState_Playing::Render_Enter() const
{
	DebugRenderSystem* system = DebugRenderSystem::GetInstance();

	system->DrawBasis(Vector3::ZERO, Vector3::ZERO, 0.f);
	system->Draw2DText(Stringf("Playing - Entering in %f", m_transitionTimer.GetTimeUntilIntervalEnds()), AABB2(Vector2(0.f, 0.f), Vector2(200.f, 200.f)), 0.f, Rgba::WHITE, 50.f, Vector2(0.f, 1.0f));
}


//-----------------------------------------------------------------------------------------------
// Checks for input
//
void GameState_Playing::ProcessInput()
{
	UpdateCameraOnInput();

	// To return to main menu
	if (InputSystem::GetInstance()->WasKeyJustPressed(InputSystem::KEYBOARD_ESCAPE))
	{
		Game::TransitionToGameState(new GameState_MainMenu());
	}
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
	DebugRenderSystem* system = DebugRenderSystem::GetInstance();

	system->DrawBasis(Vector3::ZERO, Vector3::ZERO, 0.f);
	system->Draw2DText("Playing", AABB2(Vector2(0.f, 0.f), Vector2(200.f, 200.f)), 0.f, Rgba::WHITE, 50.f, Vector2(0.f, 1.0f));
}


//-----------------------------------------------------------------------------------------------
// Renders the leave state transition
//
void GameState_Playing::Render_Leave() const
{
	DebugRenderSystem* system = DebugRenderSystem::GetInstance();

	system->DrawBasis(Vector3::ZERO, Vector3::ZERO, 0.f);
	system->Draw2DText(Stringf("Playing - Leaving in %f", m_transitionTimer.GetTimeUntilIntervalEnds()), AABB2(Vector2(0.f, 0.f), Vector2(200.f, 200.f)), 0.f, Rgba::WHITE, 50.f, Vector2(0.f, 1.0f));
}


//-----------------------------------------------------------------------------------------------
// Updates the camera based on keyboard input for no-clip flying
//
void GameState_Playing::UpdateCameraOnInput()
{
	float deltaTime = Game::GetDeltaTime();
	InputSystem* input = InputSystem::GetInstance();

	// Translating the camera
	Vector3 translationOffset = Vector3::ZERO;
	if (input->IsKeyPressed('W')) { translationOffset.z += 1.f; }									// Forward
	if (input->IsKeyPressed('S')) { translationOffset.z -= 1.f; }									// Left
	if (input->IsKeyPressed('A')) { translationOffset.x -= 1.f; }									// Back
	if (input->IsKeyPressed('D')) { translationOffset.x += 1.f; }									// Right
	if (input->IsKeyPressed(InputSystem::KEYBOARD_SPACEBAR)) { translationOffset.y += 1.f; }		// Up
	if (input->IsKeyPressed('X')) { translationOffset.y -= 1.f; }									// Down

	// Shift for faster movement
	if (input->IsKeyPressed(InputSystem::KEYBOARD_SHIFT))
	{
		translationOffset *= 10.f;
	}

	translationOffset *= CAMERA_TRANSLATION_SPEED * deltaTime;

	Camera* gameCamera = Game::GetGameCamera();
	gameCamera->TranslateLocal(translationOffset);

	// Rotating the camera
	Mouse& mouse = InputSystem::GetMouse();
	IntVector2 mouseDelta = mouse.GetMouseDelta();

	Vector2 rotationOffset = Vector2((float)mouseDelta.y, (float)mouseDelta.x) * 0.12f;
	Vector3 rotation = Vector3(rotationOffset.x * CAMERA_ROTATION_SPEED * deltaTime, rotationOffset.y * CAMERA_ROTATION_SPEED * deltaTime, 0.f);

	gameCamera->Rotate(rotation);
}
