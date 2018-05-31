/************************************************************************/
/* Project: SD1 C27 A2 - Asteroids
/* File: Ship.cpp
/* Author: Andrew Chase
/* Date: September 8th, 2017
/* Bugs: None
/* Description: Implementation of the Ship class
/************************************************************************/
#include "Game/Ship.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Input/InputSystem.hpp"


//-----------------------------------------------------------------------------------------------
// Checks to ensure the controller is connected before assuming the stored stick data is
// valid. Then checks the stored data to see if the stick was pressed (enough) this frame
//
bool ControllerStickPressed()
{
	bool isConnected = g_theInputSystem->GetController(0).IsConnected();
	bool stickPressed = (g_theInputSystem->GetController(0).GetCorrectedStickMagnitude(XBOX_STICK_LEFT) > 0);

	return (isConnected && stickPressed);
}


//-----------------------------------------------------------------------------------------------
// Default (and only) constructor used for all ships
//
Ship::Ship()
{
	// Placing ship in the center of the screen
	m_position = Vector2(500.f, 500.f);
		
	// Initial rotation is at 90 degrees to face the ship up
	m_orientationDegrees = 90.f;

	// No movement initially
	m_velocity = Vector2(0.f, 0.f);
	m_angularVelocity = 0.f;

	// Hard coding for now
	m_innerRadius = 12.f;
	m_outerRadius = 20.f;

	//-----Ship members-----
	m_isDead = false;
	m_invincibilityTimer = 3.f;
	m_age = 0.f;
	m_ageAtDeath = -1.f;
}


//-----------------------------------------------------------------------------------------------
// Updates the velocity based on the direction the ship is facing
//
void Ship::UpdateLinearVelocity(float deltaTime)
{
	// Prioritize controller input over keyboard input
	if (ControllerStickPressed())
	{
		XboxController& controller = g_theInputSystem->GetController(0);
		Vector2 shipDirection = Vector2::MakeDirectionAtDegrees(controller.GetStickOrientationDegrees(XBOX_STICK_LEFT));
		m_velocity += (shipDirection * deltaTime * BASE_SHIP_ACCELERATION * controller.GetCorrectedStickMagnitude(XBOX_STICK_LEFT));
	}
	else if (g_theInputSystem->IsKeyPressed(InputSystem::KEYBOARD_UP_ARROW) || g_theInputSystem->IsKeyPressed('E'))
	{
		Vector2 shipDirection = Vector2::MakeDirectionAtDegrees(m_orientationDegrees);
		m_velocity += (shipDirection * deltaTime * BASE_SHIP_ACCELERATION);
	}
}


//-----------------------------------------------------------------------------------------------
// Moves the ship in the direction it currently is going in
//
void Ship::UpdateAngularVelocity()
{
	
	if (ControllerStickPressed())
	{
		XboxController& controller = g_theInputSystem->GetController(0);
		m_angularVelocity = 0;
		m_orientationDegrees = controller.GetStickOrientationDegrees(XBOX_STICK_LEFT);
	}
	else
	{
		float newAngularVelocity = 0.f;

		if (g_theInputSystem->IsKeyPressed(InputSystem::KEYBOARD_LEFT_ARROW) || g_theInputSystem->IsKeyPressed('S'))
		{
			newAngularVelocity += SHIP_TURN_SPEED;
		}

		if (g_theInputSystem->IsKeyPressed(InputSystem::KEYBOARD_RIGHT_ARROW) || g_theInputSystem->IsKeyPressed('F'))
		{
			newAngularVelocity -= SHIP_TURN_SPEED;
		}

		m_angularVelocity = newAngularVelocity;
	}
}


//-----------------------------------------------------------------------------------------------
// Decrements the invincibility timer by deltaTime until it hits zero
//
void Ship::UpdateInvincibilityTimer(float deltaTime)
{
	if (m_invincibilityTimer > 0.f)
	{
		m_invincibilityTimer -= deltaTime;
	}

	// To keep it non-negative after it expires
	if (m_invincibilityTimer < 0.f)
	{
		m_invincibilityTimer = 0.f;
	}
}


//-----------------------------------------------------------------------------------------------
// Moves the ship in the direction it currently is going in
//
void Ship::Update(float deltaTime)
{
	// Update our velocity if the up arrow is pressed
	if (!m_isDead)
	{
		// Update our velocity based on whether or not the up arrow is pressed
		UpdateLinearVelocity(deltaTime);
		
		// Update the ship's angular velocity based on if the left/right arrows are pressed
		UpdateAngularVelocity();

		// Update the invincibility timer
		UpdateInvincibilityTimer(deltaTime);
	}

	// Update position and orientation
	Entity::Update(deltaTime);
}


//-----------------------------------------------------------------------------------------------
// Draws the ship to screen
//
void Ship::Render() const
{
	if (!m_isDead)
	{
		// Set up coordinate system
		g_theRenderer->PushMatrix();
		g_theRenderer->TranslateCoordinateSystem(m_position.x, m_position.y, 0.f);
		g_theRenderer->RotateCoordinateSystem2D(m_orientationDegrees);

		// Scale the ship by age initially for a cool effect
		float shipScaler = (2.f - Clamp01(m_age));

		g_theRenderer->ScaleCoordinateSystem(m_outerRadius * shipScaler);
		
		// Scale the color by age, so the ship fades in when respawned
		Rgba shipColor = Rgba::LIGHT_BLUE;
		if (IsInvincible())
		{
			int quotient = static_cast<int>(m_invincibilityTimer * FLASHES_PER_SECOND);
			// If even flash cyan, otherwise stay light blue
			if ((quotient % 2) == 0)
			{
				shipColor = Rgba::CYAN;
			}		
		}

		// Fade in effect
		shipColor.ScaleAlpha(m_age);

		// Draw the lines for the ship shape
		g_theRenderer->SetDrawColor(shipColor);

		g_theRenderer->DrawLine(Vector2(1.f, 0.f), Vector2(-0.8f, 0.6f));
		g_theRenderer->DrawLine(Vector2(-0.8f, 0.6f), Vector2(-0.4f, 0.3f));
		g_theRenderer->DrawLine(Vector2(-0.4f, 0.3f), Vector2(-0.4f, -0.3f));
		g_theRenderer->DrawLine(Vector2(-0.4f, -0.3f), Vector2(-0.8f, -0.6f));
		g_theRenderer->DrawLine(Vector2(-0.8f, -0.6f), Vector2(1.f, 0.f));

		Rgba redColor = Rgba::RED;
		redColor.ScaleAlpha(m_age);

		Rgba yellowColor = Rgba::YELLOW;
		yellowColor.ScaleAlpha(m_age);

		// If the ship is accelerating, draw the flame behind it
		bool isEPressed = g_theInputSystem->IsKeyPressed('E');
		bool isUpPressed = g_theInputSystem->IsKeyPressed(InputSystem::KEYBOARD_UP_ARROW);
		if (isUpPressed || isEPressed || ControllerStickPressed())
		{
			float randomFireLength = GetRandomFloatInRange(-0.8f, -1.0f);
			g_theRenderer->DrawLineColor(Vector2(-0.4f, 0.3f), redColor, Vector2(randomFireLength, 0.f), yellowColor);
			g_theRenderer->DrawLineColor(Vector2(-0.4f, -0.3f), redColor, Vector2(randomFireLength, 0.f), yellowColor);
		}

		// Set the color back to white
		g_theRenderer->SetDrawColor(Rgba::WHITE);

		// Reset the coordinate system
		g_theRenderer->PopMatrix();
	}
}


//-----------------------------------------------------------------------------------------------
// Returns whether or not the ship is dead
//
bool Ship::IsDead() const
{
	return m_isDead;
}


//-----------------------------------------------------------------------------------------------
// Returns whether or not the ship is invincible
//
bool Ship::IsInvincible() const
{
	return (m_invincibilityTimer > 0.f);
}

//-----------------------------------------------------------------------------------------------
// Flags the ship as being dead, so it isn't rendered or responds to player input
// Does NOT destroy the ship.
//
void Ship::SetIsDead(bool newState)
{
	m_isDead = newState;

	if (m_isDead) 
	{
		m_ageAtDeath = m_age;
	}
}


//-----------------------------------------------------------------------------------------------
// Sets the ship back to initial game state
//
void Ship::Respawn()
{
	// Placing ship in the center of the screen
	m_position = Vector2(500.f, 500.f);

	// Initial rotation is at 90 degrees to face the ship up
	m_orientationDegrees = 90.f;

	// No movement initially
	m_velocity = Vector2(0.f, 0.f);
	m_angularVelocity = 0.f;

	//-----Ship members-----
	m_isDead = false;
	m_invincibilityTimer = 3.f;
	m_age = 0.f;
	m_ageAtDeath = -1.f;
}


//-----------------------------------------------------------------------------------------------
// Applies a force to the ship in the direction opposite of where it's facing
//
void Ship::ApplyRecoil()
{
	// Recoil direction is opposite the facing direction
	Vector2 recoilDirection = (-1.f * Vector2::MakeDirectionAtDegrees(m_orientationDegrees));

	m_velocity += (recoilDirection * 50.f);
}
