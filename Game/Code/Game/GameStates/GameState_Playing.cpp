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
#include "Game/Framework/SpawnManager.hpp"
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

	for (int i = 0; i < 1; ++i)
	{
		players[i] = new Player(i);
		players[i]->SetPosition(Vector3(50.f * (float) i + 30.f, 0.f, 60.f));
		players[i]->SetTeam(ENTITY_TEAM_PLAYER);

		Game::GetWorld()->AddEntity(players[i]);
	}

	Game::GetWorld()->Inititalize();
	m_spawnManager = new SpawnManager("Data/Spawning.xml");

	m_state = PLAY_STATE_WAVE;
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
// Update for when the game is at idle
//
void GameState_Playing::Update_Idle()
{
}


//-----------------------------------------------------------------------------------------------
// Update for when a wave is currently being played
//
void GameState_Playing::Update_Wave()
{
	m_spawnManager->Update();
	Game::GetWorld()->Update();

	// Camera
	if (!m_cameraEjected)
	{
		Game::GetGameCamera()->UpdatePositionBasedOnPlayers();
	}

	if (m_spawnManager->IsCurrentWaveFinished())
	{
		TransitionToState(PLAY_STATE_REST);
	}
}


//-----------------------------------------------------------------------------------------------
// Update for the period between waves
//
void GameState_Playing::Update_Rest()
{
	if (m_restTimer.HasIntervalElapsed())
	{
		TransitionToState(PLAY_STATE_WAVE);
	}
	else
	{
		Game::GetWorld()->Update();

		// Camera
		if (!m_cameraEjected)
		{
			Game::GetGameCamera()->UpdatePositionBasedOnPlayers();
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Renders the idle state
//
void GameState_Playing::Render_Idle()
{

}


//-----------------------------------------------------------------------------------------------
// Renders the wave state (gameplay during a wave)
//
void GameState_Playing::Render_Wave()
{

}


//-----------------------------------------------------------------------------------------------
// Renders the rest state (period between waves)
//
void GameState_Playing::Render_Rest()
{

}


//-----------------------------------------------------------------------------------------------
// Performs the necessary clean up and intialization for moving into a different state
//
void GameState_Playing::TransitionToState(ePlayState state)
{
	// Exit
	switch (m_state)
	{
	case PLAY_STATE_IDLE:
		break;
	case PLAY_STATE_WAVE:
		break;
	case PLAY_STATE_REST:
		break;
	default:
		break;
	}

	// Enter
	m_state = state;
	switch (m_state)
	{
	case PLAY_STATE_IDLE:
		break;
	case PLAY_STATE_WAVE:
		m_spawnManager->StartNextWave();
		break;
	case PLAY_STATE_REST:
		m_restTimer.SetInterval(REST_INTERVAL);
		break;
	default:
		break;
	}
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
	switch (m_state)
	{
	case PLAY_STATE_IDLE:
		Update_Idle();
		break;
	case PLAY_STATE_WAVE:
		Update_Wave();
		break;
	case PLAY_STATE_REST:
		Update_Rest();
		break;
	default:
		break;
	}
}


//- C FUNCTION ----------------------------------------------------------------------------------------------
// Returns the string representative of the given enum
//
std::string GetStringForPlayState(ePlayState state)
{
	switch (state)
	{
	case PLAY_STATE_IDLE:
		return "IDLE";
		break;
	case PLAY_STATE_WAVE:
		return "WAVE";
		break;
	case PLAY_STATE_REST:
		return "REST";
		break;
	default:
		return "";
		break;
	}
}

#include "Engine/Core/Window.hpp"
#include "Engine/Rendering/DebugRendering/DebugRenderSystem.hpp"
//-----------------------------------------------------------------------------------------------
// Renders the gameplay state to screen
//
void GameState_Playing::Render() const
{
	std::string stateText = GetStringForPlayState(m_state);

	DebugRenderSystem::Draw2DText(stateText, Window::GetInstance()->GetWindowBounds(), 0.f);

	// 3D
	switch (m_state)
	{
	case PLAY_STATE_IDLE:
		break;
	case PLAY_STATE_WAVE:
		Game::GetWorld()->Render();
		break;
	case PLAY_STATE_REST:
		Game::GetWorld()->Render();
		break;
	default:
		break;
	}

	// 2D
	switch (m_state)
	{
	case PLAY_STATE_IDLE:
		break;
	case PLAY_STATE_WAVE:
		break;
	case PLAY_STATE_REST:
		break;
	default:
		break;
	}
}
