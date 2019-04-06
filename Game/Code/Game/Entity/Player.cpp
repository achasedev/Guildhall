/************************************************************************/
/* File: Player.cpp
/* Author: Andrew Chase
/* Date: April 2nd 2019
/* Description: Implementation of the player class
/************************************************************************/
#include "Game/Entity/Player.hpp"
#include "Game/Framework/Game.hpp"
#include "Game/Framework/GameCamera.hpp"
#include "Game/Framework/GameCommon.hpp"
#include "Engine/Core/Window.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/Utility/StringUtils.hpp"
#include "Engine/Core/Utility/ErrorWarningAssert.hpp"
#include "Engine/Rendering/DebugRendering/DebugRenderSystem.hpp"


//-----------------------------------------------------------------------------------------------
// Process Input
//
void Player::ProcessInput()
{
	eCameraMode currentCameraMode = Game::GetGameCamera()->GetCameraMode();
	ASSERT_OR_DIE(Game::GetGameCamera()->IsAttachedToEntity(this), "Player processing input when not possessing a camera!");

	// XY Movement
	InputSystem* input = InputSystem::GetInstance();

	Vector2 inputDirection = Vector2::ZERO;

	if (input->IsKeyPressed('W'))
	{
		inputDirection.x += 1.0f;
	}

	if (input->IsKeyPressed('S'))
	{
		inputDirection.x -= 1.0f;
	}

	if (input->IsKeyPressed('A'))
	{
		inputDirection.y += 1.0f;
	}

	if (input->IsKeyPressed('D'))
	{
		inputDirection.y -= 1.0f;
	}

	if (inputDirection != Vector2::ZERO)
	{
		float inputOrientation = inputDirection.GetOrientationDegrees();
		inputOrientation += m_transform.rotation.z;

		MoveSelfHorizontal(Vector2::MakeDirectionAtDegrees(inputOrientation));
	}

	// Z Movement
	if (m_physicsMode == PHYSICS_MODE_WALKING)
	{
		if (input->WasKeyJustPressed(' '))
		{
			Jump();
		}
	}
	else
	{
		// Fly up and down
		float zInput = 0.f;

		if (input->IsKeyPressed('E')) { zInput += 1.f; }		// Up
		if (input->IsKeyPressed('Q')) { zInput -= 1.f; }		// Down

		if (zInput != 0.f)
		{
			MoveSelfVertical(zInput);
		}
	}
	

	// Rotation - depends on the camera mode
	bool shouldRotate = false;
	switch (currentCameraMode)
	{
	case CAMERA_MODE_ATTACHED_FIRST_PERSON:
	case CAMERA_MODE_ATTACHED_FIXED_ANGLE:
		shouldRotate = true;
		break;
	case CAMERA_MODE_ATTACHED_THIRD_PERSON:
	{
		Mouse& mouse = InputSystem::GetMouse();
		if (mouse.IsButtonPressed(MOUSEBUTTON_RIGHT))
		{
			// Snap rotation to what the camera is looking at
			Vector3 cameraRotation = Game::GetGameCamera()->GetRotation();
			m_transform.rotation = cameraRotation;

			shouldRotate = true;
		}
	}
		break;
	default:
		break;
	}

	if (shouldRotate)
	{
		Mouse& mouse = InputSystem::GetMouse();
		IntVector2 delta = mouse.GetMouseDelta();
		float deltaSeconds = Game::GetDeltaTime();

		float deltaPitch = (float) delta.y * 0.12f * deltaSeconds * CAMERA_ROTATION_SPEED;
		float deltaYaw = (float) -1.0f * delta.x * 0.12f * deltaSeconds * CAMERA_ROTATION_SPEED;
		
		m_transform.rotation.z += deltaYaw;
		m_transform.rotation.y = ClampFloat(m_transform.rotation.y + deltaPitch, -85.f, 85.f);

		// Keep them between 0.f and 360.f
		m_transform.rotation.y = GetAngleBetweenMinusOneEightyAndOneEighty(m_transform.rotation.y);
		m_transform.rotation.z = GetAngleBetweenMinusOneEightyAndOneEighty(m_transform.rotation.z);
	}
}


//-----------------------------------------------------------------------------------------------
// Update
//
void Player::Update()
{
	Entity::Update();
}


//---C FUNCTION----------------------------------------------------------------------------------
// Returns the string representation of the physics mode enumeration
//
std::string GetPhysicsModeTextForMode(ePhysicsMode mode)
{
	switch (mode)
	{
	case PHYSICS_MODE_WALKING:
		return "[F3] WALKING";
		break;
	case PHYSICS_MODE_FLYING:
		return "[F3] FLYING";
		break;
	case PHYSICS_MODE_NO_CLIP:
		return "[F3] NO CLIP";
		break;
	default:
		return "YOU SHOULDN'T BE HERE!";
		break;
	}
}
//-----------------------------------------------------------------------------------------------
// Render
//
void Player::Render() const
{
	Entity::Render();

	// Debug draws for camera
	Window* window = Window::GetInstance();
	AABB2 windowBounds = window->GetWindowBounds();

	std::string physicsModeText = GetPhysicsModeTextForMode(m_physicsMode);
	Vector3 position = m_transform.position;
	Vector3 rotation = m_transform.rotation;

	std::string text = Stringf("Physics Mode: %s\nPosition: (%.2f, %.2f, %.2f)\nRotation: (%.2f, %.2f, %.2f)\nVelocity: (%.2f, %.2f, %.2f)\nForce: (%.2f, %.2f, %.2f)\nImpulse: (%.2f, %.2f, %.2f)\nOnGround: %s",
		physicsModeText.c_str(), position.x, position.y, position.z, rotation.x, rotation.y, rotation.z, m_velocity.x, m_velocity.y, m_velocity.z,
		m_force.x, m_force.y, m_force.z, m_impulse.x, m_impulse.y, m_impulse.z, (m_isOnGround ? "true" : "false"));
	
	DebugRenderSystem::Draw2DText(text, windowBounds, 0.f, Rgba::DARK_GREEN, 20.f, Vector2(1.0f, 1.0f));
}
