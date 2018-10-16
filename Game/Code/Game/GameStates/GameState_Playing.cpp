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
#include "Game/GameStates/GameState_Playing.hpp"
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

	players[0] = new Player(0);
	players[0]->SetPosition(Vector3(60.f, 0.f, 60.f));

	Game::GetWorld()->Inititalize("Data/VoxelModels/Ground.qef");

	Game::GetWorld()->AddEntity(players[0]);

	// Spawn some test entities
	for (int i = 0; i < 10; ++i)
	{
		Game::GetWorld()->AddEntity(new AIEntity(EntityDefinition::GetDefinition("Robot")));
	}

	for (int i = 0; i < 100; ++i)
	{
		Entity* newEntity = new Entity(EntityDefinition::GetDefinition("Wall"));
		newEntity->SetPosition(Vector3(GetRandomFloatInRange(0.f, 254.f), 4.f, GetRandomFloatInRange(0.f, 254.f)));
		Game::GetWorld()->AddEntity(newEntity);
	}
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

	GameCamera* gameCamera = Game::GetGameCamera();
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
	if (InputSystem::GetInstance()->WasKeyJustPressed('B'))
	{
		m_cameraEjected = !m_cameraEjected;
	}

	Player** players = Game::GetPlayers();

	for (int i = 0; i < MAX_PLAYERS; ++i)
	{
		if (players[i] != nullptr)
		{
			players[i]->ProcessInput();
		}
	}

	if (m_cameraEjected)
	{
		UpdateCameraOnInput();
	}

	if (InputSystem::GetInstance()->WasKeyJustPressed('L'))
	{
		Game::GetWorld()->ParticalizeAllEntities();
	}
}


//-----------------------------------------------------------------------------------------------
// Updates the play state
//
void GameState_Playing::Update()
{
	Game::GetWorld()->Update();

	// Camera
	if (!m_cameraEjected)
	{
		Game::GetGameCamera()->UpdatePositionBasedOnPlayers();
	}
}


//-----------------------------------------------------------------------------------------------
// Renders the gameplay state to screen
//
void GameState_Playing::Render() const
{
	Game::GetWorld()->Render();
}
