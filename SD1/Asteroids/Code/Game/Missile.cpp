/************************************************************************/
/* Project: MP1 C27 A1 - Vector2, Basics, MathUtils
/* File: Missile.cpp
/* Author: Andrew Chase
/* Date: September 21st, 2017
/* Bugs: None
/* Description: Implementation of the Missile class
/************************************************************************/
#include "Game/Missile.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Vector2.hpp"


//-----------------------------------------------------------------------------------------------
// Constructs a missile similar to how a bullet is constructed, given the ship's information and
// a target
//
Missile::Missile(Vector2 nosePosition, Vector2 fireDirection, float shipOrientation, Asteroid* target)
{
	m_position = nosePosition;
	m_velocity = (BASE_MISSILE_SPEED * Vector2::MakeDirectionAtDegrees(shipOrientation));

	m_angularVelocity = 0.f;
	m_orientationDegrees = shipOrientation;

	m_innerRadius = 12.f;
	m_outerRadius = 15.f;

	// Set the target
	m_target = target;
	if (m_target != nullptr)
	{
		m_hasTarget = true;
	}
}


//-----------------------------------------------------------------------------------------------
// Updates the missile's trajectory towards its target, and then calls the inherited Update() 
// from Entity
//
void Missile::Update(float deltaTime)
{
	if (m_hasTarget)
	{
		TurnTowardsTarget(deltaTime);
	}
	
	// Update velocity - accelerate towards target
	Vector2 direction = (m_target->GetPosition() - m_position).GetNormalized();
	m_velocity += (MISSILE_ACCELERATION * deltaTime * direction);

	Entity::Update(deltaTime);
}


//-----------------------------------------------------------------------------------------------
// Rotates the missile towards it's target, in the shortest direction
//
void Missile::TurnTowardsTarget(float deltaTime)
{
	Vector2 directionToTarget = (m_target->GetPosition() - m_position);
	float angleToTarget = directionToTarget.GetOrientationDegrees();

	int directionScalar = GetShortestRotationDirection2D(m_orientationDegrees, angleToTarget);

	m_orientationDegrees += (MISSILE_TURN_SPEED * directionScalar * deltaTime);
}


//-----------------------------------------------------------------------------------------------
// Draws the missile to screen
//
void Missile::Render() const
{
	// Set up coordinate system
	g_theRenderer->PushMatrix();
	g_theRenderer->TranslateCoordinateSystem(m_position.x, m_position.y, 0.f);
	g_theRenderer->RotateCoordinateSystem2D(m_orientationDegrees);
	g_theRenderer->ScaleCoordinateSystem(m_outerRadius - 1.f);

	//-----Drawing the lines for the missile shape-----

	// Red Cone
	g_theRenderer->DrawLineColor(Vector2(1.f, 0.f), Rgba::RED, Vector2(0.4f, 0.4f), Rgba::RED);
	g_theRenderer->DrawLineColor(Vector2(1.f, 0.f), Rgba::RED, Vector2(0.4f, -0.4f), Rgba::RED);
	g_theRenderer->DrawLineColor(Vector2(0.4f, 0.4f), Rgba::RED, Vector2(0.4f, -0.4f), Rgba::RED);

	// White Center
	g_theRenderer->DrawLineColor(Vector2(0.4f, 0.2f), Rgba::WHITE, Vector2(-0.4f, 0.2f), Rgba::WHITE);
	g_theRenderer->DrawLineColor(Vector2(0.4f, -0.2f), Rgba::WHITE, Vector2(-0.4f, -0.2f), Rgba::WHITE);

	// Blue Base
	g_theRenderer->DrawLineColor(Vector2(-0.4f, 0.2f), Rgba::BLUE, Vector2(-1.f, 0.6f), Rgba::BLUE);
	g_theRenderer->DrawLineColor(Vector2(-0.4f, -0.2f), Rgba::BLUE, Vector2(-1.f, -0.6f), Rgba::BLUE);

	g_theRenderer->DrawLineColor(Vector2(-1.f, 0.2f), Rgba::BLUE, Vector2(-1.f, 0.6f), Rgba::BLUE);
	g_theRenderer->DrawLineColor(Vector2(-1.f, -0.2f), Rgba::BLUE, Vector2(-1.f, -0.6f), Rgba::BLUE);


	g_theRenderer->DrawLineColor(Vector2(-1.f, 0.2f), Rgba::BLUE, Vector2(-0.8f, 0.2f), Rgba::BLUE);
	g_theRenderer->DrawLineColor(Vector2(-1.f, -0.2f), Rgba::BLUE, Vector2(-0.8f, -0.2f), Rgba::BLUE);

	g_theRenderer->DrawLineColor(Vector2(-0.8f, 0.2f), Rgba::BLUE, Vector2(-0.8f, -0.2f), Rgba::BLUE);

	// Flame
	float randomFireLength = GetRandomFloatInRange(-1.f, -1.2f);
	g_theRenderer->DrawLineColor(Vector2(-0.8f, 0.2f), Rgba::RED, Vector2(randomFireLength, 0.f), Rgba::YELLOW);
	g_theRenderer->DrawLineColor(Vector2(-0.8f, -0.2f), Rgba::RED, Vector2(randomFireLength, 0.f), Rgba::YELLOW);

	// Reset the coordinate system
	g_theRenderer->PopMatrix();
}


//-----------------------------------------------------------------------------------------------
// Sets the target of the missile to the newTarget
//
void Missile::SetTarget(Asteroid* newTarget)
{
	m_target = newTarget;
}
