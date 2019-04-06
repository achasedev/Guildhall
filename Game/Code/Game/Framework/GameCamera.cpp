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
const Vector3 GameCamera::CAMERA_FIXED_ANGLE_DIRECTION = Vector3(-0.651f, -0.651f, 0.3906f);


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
		ProcessInputFixedAngle();
		break;
	case CAMERA_MODE_ATTACHED_FIRST_PERSON:
	default:
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
	CheckIfEntityStillValid();

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
// Allows for zooming in and out while at a fixed angle to an entity
//
void GameCamera::ProcessInputFixedAngle()
{
	Mouse& mouse = InputSystem::GetMouse();
	IntVector2 delta = mouse.GetMouseDelta();

	float zoomDelta = mouse.GetMouseWheelDelta();
	m_offsetMagnitude = ClampFloat(m_offsetMagnitude - zoomDelta, CAMERA_THIRD_PERSON_MIN_DISTANCE, CAMERA_THIRD_PERSON_MAX_DISTANCE);
}


//-----------------------------------------------------------------------------------------------
// Checks for input in rotating the camera around the entity as an orbit camera
//
void GameCamera::ProcessInputThirdPerson()
{
	float deltaSeconds = Game::GetDeltaTime();
	Mouse& mouse = InputSystem::GetMouse();

	if (!mouse.IsButtonPressed(MOUSEBUTTON_RIGHT))
	{
		IntVector2 delta = mouse.GetMouseDelta();
		Vector2 rotationOffset = Vector2((float)delta.y, (float)delta.x) * 0.12f;
		m_frameRotation = Vector3(0.f, -1.0f * rotationOffset.x * CAMERA_ROTATION_SPEED * deltaSeconds, rotationOffset.y * CAMERA_ROTATION_SPEED * deltaSeconds);
	}

	float zoomDelta = mouse.GetMouseWheelDelta();
	m_offsetMagnitude = ClampFloat(m_offsetMagnitude - zoomDelta, CAMERA_THIRD_PERSON_MIN_DISTANCE, CAMERA_THIRD_PERSON_MAX_DISTANCE);
}


//-----------------------------------------------------------------------------------------------
// Checks if the entity the camera is attached to still exists, in case it was deleted
//
void GameCamera::CheckIfEntityStillValid()
{
	if (m_cameraMode != CAMERA_MODE_DETACHED && m_entityAttachedTo->IsMarkedForDelete())
	{
		m_entityAttachedTo = nullptr;
		m_cameraMode = CAMERA_MODE_DETACHED;
	}
}


//-----------------------------------------------------------------------------------------------
// Updates the camera's transform to match that of the entity's that it is attached to
//
void GameCamera::UpdateFirstPerson()
{
	// Set the position
	Vector3 eyePosition = m_entityAttachedTo->GetEyeWorldPosition();
	SetPosition(eyePosition);

	Vector3 cameraRotation = Vector3::ZERO;

	// Make the XY rotation match the entity's
	float yawOrientationDegrees = m_entityAttachedTo->GetYawOrientationDegrees();
	float pitchOrientationDegrees = m_entityAttachedTo->GetPitchOrientationDegrees();

	cameraRotation.y = pitchOrientationDegrees;
	cameraRotation.z = yawOrientationDegrees;

	SetRotation(cameraRotation);

	// Zero them out for next frame
	m_frameRotation = Vector3::ZERO;
}


//-----------------------------------------------------------------------------------------------
// Updates the camera's transform to look at the entity from behind
//
void GameCamera::UpdateThirdPerson()
{
	// Get the "eye" transform of the entity and assign it to the camera
	Vector3 entityEyePosition = m_entityAttachedTo->GetEyeWorldPosition();

	// Update the rotation either:
	//  1. If the right button was pressed, we snap to the entity and do no rotation ourselves
	//  2. If the right button was not pressed, we rotate ourselves

	Mouse& mouse = InputSystem::GetMouse();
	if (!mouse.IsButtonPressed(MOUSEBUTTON_RIGHT))
	{
		// Update the spherical rotation to avoid snapping
		if (mouse.WasButtonJustReleased(MOUSEBUTTON_RIGHT))
		{
			Vector3 entityForward = m_entityAttachedTo->GetForwardVector();
			Vector3 c = -1.0f * entityForward;

			float theta = Atan2Degrees(c.x / c.y);
			float phi = ACosDegrees(c.z / c.GetLength());

			if (GetRotation().z > 0.f)
			{
				theta += 180.f;
			}

			m_orbitSphericalRotation.y = GetAngleBetweenZeroThreeSixty(phi);
			m_orbitSphericalRotation.z = GetAngleBetweenZeroThreeSixty(theta);
		}

		m_orbitSphericalRotation += m_frameRotation;
		m_orbitSphericalRotation.y = ClampFloat(m_orbitSphericalRotation.y, 10.f, 170.f);

		float yawRotation = m_orbitSphericalRotation.z;
		float pitchRotation = m_orbitSphericalRotation.y;

		// Find the position of the camera in world coordinates
		Vector3 cameraOffset;

		cameraOffset.y = m_offsetMagnitude * CosDegrees(yawRotation) * SinDegrees(pitchRotation);
		cameraOffset.x = m_offsetMagnitude * SinDegrees(yawRotation) * SinDegrees(pitchRotation);
		cameraOffset.z = m_offsetMagnitude * CosDegrees(pitchRotation);

		LookAt(cameraOffset + entityEyePosition, entityEyePosition, Vector3::Z_AXIS);

		m_frameRotation = Vector3::ZERO;
	}
	else
	{
		Vector3 entityForward = m_entityAttachedTo->GetForwardVector();

		Vector3 cameraOffset = -1.0f * entityForward * m_offsetMagnitude;
		LookAt(entityEyePosition + cameraOffset, entityEyePosition, Vector3::Z_AXIS);
	}
}


//-----------------------------------------------------------------------------------------------
// Updates the camera to look at the entity from a fixed angle
//
void GameCamera::UpdateFixedAngle()
{
	Vector3 entityCenterPosition = m_entityAttachedTo->GetCenterWorldPosition();
	Vector3 cameraPosition = entityCenterPosition + CAMERA_FIXED_ANGLE_DIRECTION * m_offsetMagnitude;
	LookAt(cameraPosition, entityCenterPosition, Vector3::Z_AXIS);
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
