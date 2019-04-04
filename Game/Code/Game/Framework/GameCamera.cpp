/************************************************************************/
/* File: GameCamera.cpp
/* Author: Andrew Chase
/* Date: February 9th 2019
/* Description: Implementation of the GameCamera class
/************************************************************************/
#include "Game/Entity/Entity.hpp"
#include "Game/Framework/Game.hpp"
#include "Game/Framework/GameCamera.hpp"
#include "Game/Framework/GameCommon.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Input/InputSystem.hpp"


// Statics
const Vector3 GameCamera::CAMERA_FIXED_ANGLE_OFFSET = Vector3(-10.f, 0.f, 10.f);


//-----------------------------------------------------------------------------------------------
// Checks for keyboard/mouse input used to move the camera depending on the mode
//
void GameCamera::ProcessInput()
{
	switch (m_cameraMode)
	{
	case CAMERA_MODE_ATTACHED_THIRD_PERSON:
		ProcessInputThirdPerson();
		break;
	case CAMERA_MODE_DETACHED:
		ProcessInputDetached();
		break;
	case CAMERA_MODE_ATTACHED_FIXED_ANGLE:
	case CAMERA_MODE_ATTACHED_FIRST_PERSON:
	default:
		// Fixed angle and first person don't listen for input, camera just "copies" the entity in Update()
		break;
	}
}


//-----------------------------------------------------------------------------------------------
// Updates the camera by translating/rotating it by the frame translations and rotations
// Depending on the mode, these frame deltas are either determined in the mode's process input or
// in the mode's update right now
//
void GameCamera::Update()
{
	switch (m_cameraMode)
	{
	case CAMERA_MODE_ATTACHED_FIRST_PERSON:
		UpdateFirstPerson();
		break;
	case CAMERA_MODE_ATTACHED_THIRD_PERSON:
		UpdateThirdPerson();
		break;
	case CAMERA_MODE_ATTACHED_FIXED_ANGLE:
		UpdateFixedAngle();
		break;
	case CAMERA_MODE_DETACHED:
		UpdateDetached();
		break;
	default:
		break;
	}
}


//-----------------------------------------------------------------------------------------------
// Attaches the camera to the given entity so it will update and track it
//
void GameCamera::AttachToEntity(Entity* entity, eCameraMode mode)
{
	ASSERT_OR_DIE(entity != nullptr, "Camera attached to null entity");
	ASSERT_OR_DIE(mode != CAMERA_MODE_DETACHED, "Attach to entity called with detached mode");
	
	m_entityAttachedTo = entity;
	m_cameraMode = mode;
}


//-----------------------------------------------------------------------------------------------
// Detaches the camera from any entity it may be attached to
//
void GameCamera::Detach()
{
	m_entityAttachedTo = nullptr;
	m_cameraMode = CAMERA_MODE_DETACHED;
}


//-----------------------------------------------------------------------------------------------
// Checks for keyboard/mouse input used to move the camera around the entity it is attached to
// Only determines the rotation to move to about the entity, UpdateThirdPerson determines the translation
//
void GameCamera::ProcessInputThirdPerson()
{
}


//-----------------------------------------------------------------------------------------------
// Checks for keyboard/mouse input used to move the camera by itself, and stores it off for update
//
void GameCamera::ProcessInputDetached()
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
// Updates the camera's transform to match that of the entity's that it is attached to
//
void GameCamera::UpdateFirstPerson()
{
}


//-----------------------------------------------------------------------------------------------
// Updates the camera's transform to look at the entity as an orbit camera, using the rotation
// offset found in ProcessInputThirdPerson
//
void GameCamera::UpdateThirdPerson()
{
}


//-----------------------------------------------------------------------------------------------
// Updates the camera to look at the entity from a fixed angle
//
void GameCamera::UpdateFixedAngle()
{
	Vector3 entityPosition = m_entityAttachedTo->GetPosition();
	Vector3 cameraPosition = entityPosition + CAMERA_FIXED_ANGLE_OFFSET;
	LookAt(cameraPosition, entityPosition, Vector3::Z_AXIS);
}


//-----------------------------------------------------------------------------------------------
// Applies the frame translation and rotation set from ProcessInputDetached
//
void GameCamera::UpdateDetached()
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
