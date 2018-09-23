/************************************************************************/
/* File: Player.cpp
/* Author: Andrew Chase
/* Date: September 22nd, 2017
/* Description: Implementation of the player class
/************************************************************************/
#include "Game/Entity/Player.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Math/Vector3.hpp"

#include "Engine/Rendering/DebugRendering/DebugRenderSystem.hpp"
#include "Engine/Core/Utility/StringUtils.hpp"
#include "Engine/Math/MathUtils.hpp"

//-----------------------------------------------------------------------------------------------
// Constructor
//
Player::Player(unsigned int playerID)
	: DynamicEntity()
	, m_playerID(playerID)
{
}


//-----------------------------------------------------------------------------------------------
// Destructor
//
Player::~Player()
{
}


//-----------------------------------------------------------------------------------------------
// Checks for player related input on the respective controller
//
void Player::ProcessInput()
{
	XboxController& controller = InputSystem::GetInstance()->GetController(m_playerID);

	Vector2 leftStick = controller.GetCorrectedStickPosition(XBOX_STICK_LEFT);

	const float maxSpeed = 20.f;
	float currSpeed = m_velocity.GetLength();

	if (currSpeed < maxSpeed)
	{
		AddForce(Vector3(leftStick.x, 0.f, leftStick.y) * 200.f);
	}
	//AddForce(Vector3(leftStick.x, 0.f, leftStick.y) * 200.f);



	if (leftStick.GetLengthSquared() > 0.f)
	{
		m_orientation = leftStick.GetOrientationDegrees();
	}

	if (controller.WasButtonJustPressed(XBOX_BUTTON_X))
	{
		AddForce(Vector3(leftStick.x, 0.f, leftStick.y) * 300.f);
	}
}


//-----------------------------------------------------------------------------------------------
// Update
//
void Player::Update()
{
	DynamicEntity::Update();

	DebugRenderSystem::Draw2DText(Stringf("Force: %f\nAcceleration: %f\nVelocity: %f", m_force.GetLength(), m_acceleration.GetLength(), m_velocity.GetLength()), AABB2(Vector2::ZERO, Vector2(1600.f, 800.f)), 0.f);
}


//-----------------------------------------------------------------------------------------------
// Collision callback
//
void Player::OnCollision(Entity* other)
{
	DynamicEntity::OnCollision(other);
}
