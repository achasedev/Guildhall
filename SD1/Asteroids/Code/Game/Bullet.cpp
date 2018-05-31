/************************************************************************/
/* Project: SD1 C27 A2 - Asteroids Playable
/* File: Bullet.cpp
/* Author: Andrew Chase
/* Date: September 9th, 2017
/* Bugs: None
/* Description: Implementation of the Bullet class
/************************************************************************/
#include "Game/Bullet.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Math/MathUtils.hpp"


const float Bullet::BULLET_SPEED = 500.f;	// Default speed all bullets fly at


//-----------------------------------------------------------------------------------------------
// Only constructor for a Bullet - used to set the initial position and velocity to its correct values
//
Bullet::Bullet(Vector2 nosePosition, Vector2 fireDirection, Vector2 shipVelocity, float shipOrientation)
{
	// Spawn the bullet at the nose of the ship
	m_position = nosePosition;

	// Fire the bullet in the direction the ship's facing, adding
	// on the ship's velocity
	m_velocity = (fireDirection * BULLET_SPEED) + shipVelocity;

	m_angularVelocity = 0.f;
	m_orientationDegrees = shipOrientation;

	// Since the Bullet is small, keep the inner/outer radii equal
	m_innerRadius = 5.f;
	m_outerRadius = 5.f;

	m_age = 0.f;
	m_ageAtDeath = -1.f;

	// bullets live for two seconds
	m_timeToLive = 2.f;

}


//-----------------------------------------------------------------------------------------------
// Update the Bullet's position every frame based on its velocity, and decrement its time to live
//
void Bullet::Update(float deltaTime)
{
	Entity::Update(deltaTime);
	m_timeToLive -= deltaTime;
}


//-----------------------------------------------------------------------------------------------
// Draw the bullet to screen as a teardrop with a red-yellow color gradient
//
void Bullet::Render() const
{
	
	// Set up coordinate system
	g_theRenderer->PushMatrix();
	g_theRenderer->TranslateCoordinateSystem(m_position.x, m_position.y, 0.f);
	g_theRenderer->RotateCoordinateSystem2D(m_orientationDegrees);

	// Scale the ship by age initially for a cool effect
	float bulletScaler = (2.f - Clamp01(m_age));

	g_theRenderer->ScaleCoordinateSystem(m_outerRadius * bulletScaler);

	// Draw the lines for the bullet shape
	g_theRenderer->DrawLineColor(Vector2(1.f, 0.f), Rgba::RED, Vector2(0.5f, 0.5f), Rgba::RED);
	g_theRenderer->DrawLineColor(Vector2(0.5f, 0.5f), Rgba::RED, Vector2(-1.f, 0.f), Rgba::YELLOW);

	g_theRenderer->DrawLineColor(Vector2(1.f, 0.f), Rgba::RED, Vector2(0.5f, -0.5f), Rgba::RED);
	g_theRenderer->DrawLineColor(Vector2(0.5f, -0.5f), Rgba::RED, Vector2(-1.f, 0.f), Rgba::YELLOW);

	// Reset the coordinate system
	g_theRenderer->PopMatrix();
}


//-----------------------------------------------------------------------------------------------
// Returns true if the bullet has been in the game for at least two seconds
//
bool Bullet::IsExpired() const
{
	return (m_timeToLive <= 0.f);
}
