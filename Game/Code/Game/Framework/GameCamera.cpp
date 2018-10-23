/************************************************************************/
/* File: GameCamera.cpp
/* Author: Andrew Chase
/* Date: September 28th 2018
/* Description: Implementation of the GameCamera class
/************************************************************************/
#include "Game/Entity/Player.hpp"
#include "Game/Framework/Game.hpp"
#include "Game/Framework/GameCamera.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Input/InputSystem.hpp"

//-----------------------------------------------------------------------------------------------
// Constructor
//
GameCamera::GameCamera()
	: m_offsetDirection(Vector3(0.f, 0.75f, -1.f))
	, m_offsetDistance(150.f)
{
}


//-----------------------------------------------------------------------------------------------
// Destructor
//
GameCamera::~GameCamera()
{

}


//-----------------------------------------------------------------------------------------------
// Updates the camera to be positioned relative to the players' locations
//
void GameCamera::UpdatePositionBasedOnPlayers()
{
	Player** players = Game::GetPlayers();

	// Average the player's locations
	Vector3 targetPos = Vector3::ZERO;
	float playerCount = 0.f;

	for (int i = 0; i < MAX_PLAYERS; ++i)
	{
		if (players[i] != nullptr)
		{
			targetPos += players[i]->GetEntityPosition();
			playerCount += 1.0f;
		}
	}

	targetPos /= playerCount;

	Vector3 newPos = targetPos + m_offsetDirection * m_offsetDistance;
	LookAt(newPos, targetPos);
}


//-----------------------------------------------------------------------------------------------
// Updates the camera by checking for input
//
void GameCamera::UpdatePositionOnInput()
{
	float deltaTime = Game::GetDeltaTime();
	InputSystem* input = InputSystem::GetInstance();

	// Translating the camera
	Vector3 translationOffset = Vector3::ZERO;
	if (input->IsKeyPressed('W')) { translationOffset.z += 1.f; }		// Forward
	if (input->IsKeyPressed('S')) { translationOffset.z -= 1.f; }		// Left
	if (input->IsKeyPressed('A')) { translationOffset.x -= 1.f; }		// Back
	if (input->IsKeyPressed('D')) { translationOffset.x += 1.f; }		// Right
	if (input->IsKeyPressed(InputSystem::KEYBOARD_SPACEBAR)) { translationOffset.y += 1.f; }		// Up
	if (input->IsKeyPressed('X')) { translationOffset.y -= 1.f; }		// Down

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

	Vector2 rotationOffset = Vector2((float)mouseDelta.y, (float)mouseDelta.x) * 0.12f;
	Vector3 rotation = Vector3(rotationOffset.x * CAMERA_ROTATION_SPEED * deltaTime, rotationOffset.y * CAMERA_ROTATION_SPEED * deltaTime, 0.f);

	gameCamera->Rotate(rotation);
}


//-----------------------------------------------------------------------------------------------
// Toggles the ejected state of the camera
//
void GameCamera::ToggleEjected()
{
	m_cameraEjected = !m_cameraEjected;
}


//-----------------------------------------------------------------------------------------------
// Sets the ejected state of the camera to the state provided
//
void GameCamera::SetEjected(bool newState)
{
	m_cameraEjected = newState;
}


//-----------------------------------------------------------------------------------------------
// Returns the ejected state of the camera
//
bool GameCamera::IsEjected() const
{
	return m_cameraEjected;
}
