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


}


//-----------------------------------------------------------------------------------------------
// Update
//
void Player::Update()
{
	Entity::Update();
}


//-----------------------------------------------------------------------------------------------
// Render
//
void Player::Render() const
{
	Entity::Render();
}
