/************************************************************************/
/* File: Player.cpp
/* Author: Andrew Chase
/* Date: May 9th, 2018
/* Description: Implementation of the player class
/************************************************************************/
#include "Game/Player.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Input/InputSystem.hpp"

// Class constants
const float Player::PLAYER_LOOK_SENSITIVITY = 45.f;
const float Player::PLAYER_MOVE_SPEED = 20.f;


//-----------------------------------------------------------------------------------------------
// Constructor
//
Player::Player()
{
}


//-----------------------------------------------------------------------------------------------
// Input function - used to update the player's position and rotation
//
void Player::ProcessInput(float deltaTime)
{
	InputSystem* input = InputSystem::GetInstance();

	// Translating the Camera
	Vector3 translationOffset = Vector3::ZERO;
	if (input->IsKeyPressed('W'))								{ translationOffset.z += 1.f; }		// Forward
	if (input->IsKeyPressed('S'))								{ translationOffset.z -= 1.f; }		// Left
	if (input->IsKeyPressed('A'))								{ translationOffset.x -= 1.f; }		// Back
	if (input->IsKeyPressed('D'))								{ translationOffset.x += 1.f; }		// Right

	// Get the translation on the 2D plane
	Vector3 worldOffset = (transform.GetModelMatrix() * Vector4(translationOffset, 0.f)).xyz();
	worldOffset.y = 0.f;

    transform.TranslateWorld(worldOffset * PLAYER_MOVE_SPEED * deltaTime);

	// Rotating the Camera
	Mouse& mouse = InputSystem::GetMouse();
	IntVector2 mouseDelta = mouse.GetMouseDelta();

	Vector2 rotationOffset = Vector2((float) mouseDelta.y, (float) mouseDelta.x) * 0.12f;
	transform.Rotate(Vector3(rotationOffset.x * PLAYER_LOOK_SENSITIVITY, rotationOffset.y * PLAYER_LOOK_SENSITIVITY, 0.f) * deltaTime);
}


//-----------------------------------------------------------------------------------------------
// Update
//
void Player::Update(float deltaTime)
{
	GameObject::Update(deltaTime);
}
