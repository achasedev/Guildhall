/************************************************************************/
/* File: Player.cpp
/* Author: Andrew Chase
/* Date: April 2nd 2019
/* Description: Implementation of the player class
/************************************************************************/
#include "Game/Entity/Player.hpp"
#include "Engine/Input/InputSystem.hpp"


//-----------------------------------------------------------------------------------------------
// Process Input
//
void Player::ProcessInput()
{
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
		MoveSelfHorizontal(inputDirection.GetNormalized());
	}

	if (input->WasKeyJustPressed(' '))
	{
		Jump();
	}

	if (input->WasKeyJustPressed('J'))
	{
		m_position.z = 255.f;
	}
}


//-----------------------------------------------------------------------------------------------
// Update
//
void Player::Update()
{
	Entity::Update();
}

#include "Engine/Core/Window.hpp"
#include "Game/Framework/GameCamera.hpp"
#include "Engine/Rendering/DebugRendering/DebugRenderSystem.hpp"
#include "Engine/Core/Utility/StringUtils.hpp"

//-----------------------------------------------------------------------------------------------
// Render
//
void Player::Render() const
{
	Entity::Render();

	// Debug draws for camera
	Window* window = Window::GetInstance();
	AABB2 windowBounds = window->GetWindowBounds();

	std::string text = Stringf("Position: (%.2f, %.2f, %.2f)\nVelocity: (%.2f, %.2f, %.2f)\nForce: (%.2f, %.2f, %.2f)\nImpulse: (%.2f, %.2f, %.2f)\nOnGround: %s",
		m_position.x, m_position.y, m_position.z, m_velocity.x, m_velocity.y, m_velocity.z,
		m_force.x, m_force.y, m_force.z, m_impulse.x, m_impulse.y, m_impulse.z, (m_isOnGround ? "true" : "false"));
	
	DebugRenderSystem::Draw2DText(text, windowBounds, 0.f, Rgba::DARK_GREEN, 20.f, Vector2(1.0f, 1.0f));
}
