/************************************************************************/
/* File: Player.cpp
/* Author: Andrew Chase
/* Date: September 22nd, 2017
/* Description: Implementation of the player class
/************************************************************************/
#include "Game/Entity/Player.hpp"
#include "Game/Framework/Game.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/Utility/StringUtils.hpp"
#include "Engine/Rendering/DebugRendering/DebugRenderSystem.hpp"


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

	float deltaTime = Game::GetDeltaTime();

	float currSpeed = m_velocity.GetLength();
	Vector3 inputDirection = Vector3(leftStick.x, 0.f, leftStick.y);
	//Vector3 inputDirection = Vector3(1.0f, 0.f, 0.f);

	if (inputDirection != Vector3::ZERO)
	{
		Vector3 finalVelocity = (m_velocity + (m_maxMoveAcceleration * deltaTime) * inputDirection);
		float finalSpeed = finalVelocity.NormalizeAndGetLength();

		finalSpeed = (currSpeed > m_maxMoveSpeed ? ClampFloat(finalSpeed, 0.f, currSpeed) : ClampFloat(finalSpeed, 0.f, m_maxMoveSpeed));

		finalVelocity *= finalSpeed;

		Vector3 inputVelocityResult = finalVelocity - m_velocity;

		Vector3 acceleration = inputVelocityResult / deltaTime;

		Vector3 force = acceleration * m_mass;
		AddForce(force);	
	}

	// If we have no input or are moving too fast, decelerate
	if (leftStick == Vector2::ZERO || (currSpeed > m_maxMoveSpeed))
	{
		float amountCanBeDecreased = currSpeed;

		if (amountCanBeDecreased > 0.f)
		{
			Vector3 direction = -1.0f * m_velocity.GetNormalized();

			float decelMag = amountCanBeDecreased / deltaTime;
			decelMag = ClampFloat(decelMag, 0.f, m_maxMoveDeceleration);

			float forceMag = decelMag * m_mass;

			Vector3 force = forceMag * direction;
			AddForce(force);
		}
	}

	if (leftStick.GetLengthSquared() > 0.f)
	{
		m_orientation = leftStick.GetOrientationDegrees();
	}

	if (controller.WasButtonJustPressed(XBOX_BUTTON_X))
	{
		AddForce(inputDirection * -10000.f);
	}
}


//-----------------------------------------------------------------------------------------------
// Update
//
void Player::Update()
{
	DynamicEntity::Update();
	DebugRenderSystem::Draw2DText(Stringf("Force: %f\nAcceleration: %f\nVelocity: %f", m_force.GetLength(), m_force.GetLength() * m_inverseMass, m_velocity.GetLength()), AABB2(Vector2::ZERO, Vector2(1600.f, 800.f)), 0.f);
}


//-----------------------------------------------------------------------------------------------
// Collision callback
//
void Player::OnCollision(Entity* other)
{
	DynamicEntity::OnCollision(other);
}
