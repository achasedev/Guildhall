/************************************************************************/
/* File: GameCamera.cpp
/* Author: Andrew Chase
/* Date: February 9th 2019
/* Description: Implementation of the GameCamera class
/************************************************************************/
#include "Game/Framework/Game.hpp"
#include "Game/Framework/GameCamera.hpp"
#include "Game/Framework/GameCommon.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Input/InputSystem.hpp"


//-----------------------------------------------------------------------------------------------
// Checks for keyboard/mouse input used to move the camera, and stores it off for update
//
void GameCamera::ProcessInput()
{
	InputSystem* input = InputSystem::GetInstance();
	Mouse& mouse = InputSystem::GetMouse();

	float deltaTime = Game::GetDeltaTime();
	IntVector2 delta = mouse.GetMouseDelta();

	// Translating the camera
	m_frameTranslation = Vector3::ZERO;
	if (input->IsKeyPressed('W')) { m_frameTranslation.x += 1.f; }		// Forward
	if (input->IsKeyPressed('A')) { m_frameTranslation.y += 1.f; }		// Left
	if (input->IsKeyPressed('S')) { m_frameTranslation.x -= 1.f; }		// Back
	if (input->IsKeyPressed('D')) { m_frameTranslation.y -= 1.f; }		// Right

	m_frameTranslation = GetCameraMatrix().TransformVector(m_frameTranslation).xyz();
	m_frameTranslation.z = 0.f;
	m_frameTranslation.NormalizeAndGetLength();

	if (input->IsKeyPressed('E')) { m_frameTranslation.z += 1.f; }		// Up
	if (input->IsKeyPressed('Q')) { m_frameTranslation.z -= 1.f; }		// Down

	if (input->IsKeyPressed(InputSystem::KEYBOARD_SHIFT))
	{
		m_frameTranslation *= 8.f;
	}
	else if (input->IsKeyPressed(InputSystem::KEYBOARD_SPACEBAR))
	{
		m_frameTranslation *= 0.25f;
	}

	m_frameTranslation *= CAMERA_TRANSLATION_SPEED * deltaTime;

	Vector2 rotationOffset = Vector2((float)delta.y, (float)delta.x) * 0.12f;
	m_frameRotation = Vector3(0.f, rotationOffset.x * CAMERA_ROTATION_SPEED * deltaTime, -1.0f * rotationOffset.y * CAMERA_ROTATION_SPEED * deltaTime);
}


//-----------------------------------------------------------------------------------------------
// Updates the camera by translating/rotating it by the frame translations and rotations already
// determined by input
//
void GameCamera::Update()
{
	TranslateWorld(m_frameTranslation);	

	Vector3 cameraRotation = Rotate(m_frameRotation);

	// Clamp to avoid going upside-down
	cameraRotation.x = GetAngleBetweenMinusOneEightyAndOneEighty(cameraRotation.x);
	cameraRotation.y = GetAngleBetweenMinusOneEightyAndOneEighty(cameraRotation.y);
	cameraRotation.z = GetAngleBetweenMinusOneEightyAndOneEighty(cameraRotation.z);

	cameraRotation.y = ClampFloat(cameraRotation.y, -85.f, 85.f);
	SetRotation(cameraRotation);

	// Zero them out for next frame
	m_frameTranslation = Vector3::ZERO;
	m_frameRotation = Vector3::ZERO;
}
