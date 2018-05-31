/************************************************************************/
/* Project: SD1 C27 A2 - Asteroids
/* File: Entity.cpp
/* Author: Andrew Chase
/* Date: September 7th, 2017
/* Bugs: None
/* Description: Implementation of the Entity class
/************************************************************************/
#include "Game/Entity.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/Renderer.hpp"


void Entity::Update(float deltaTime)
{
	// Translate the Entity based on its velocity and time from the last frame
	m_position += (m_velocity * deltaTime);

	// Rotate the Entity based on its angular velocity
	m_orientationDegrees += (m_angularVelocity * deltaTime);

	// Ensure wrap around if the degree measure goes over 360
	while (m_orientationDegrees >= 360.f)
	{
		m_orientationDegrees -= 360.f;
	}

	// Update the Entity's age
	m_age += deltaTime;
}


//-----------------------------------------------------------------------------------------------
// Sets the 2D world position of the object
//
void Entity::SetPosition(const Vector2& newPosition)
{
	m_position = newPosition;
}


//-----------------------------------------------------------------------------------------------
// Sets the 2D world velocity of the object
//
void Entity::SetVelocity(const Vector2& newVelocity)
{
	m_velocity = newVelocity;
}


//-----------------------------------------------------------------------------------------------
// Sets the rotation angle of the object of the object
//
void Entity::SetOrientationDegrees(float newOrientationDegress)
{
	m_orientationDegrees = newOrientationDegress;
}


//-----------------------------------------------------------------------------------------------
// Sets the rotation velocity of the object (to update rotation orientation every frame)
//
void Entity::SetAngularVelocity(float newAngularVelocity)
{
	m_angularVelocity = newAngularVelocity;
}


//-----------------------------------------------------------------------------------------------
// Sets the physics radius of the object
//
void Entity::SetInnerRadius(float newInnerRadius)
{
	m_innerRadius = newInnerRadius;
}


//-----------------------------------------------------------------------------------------------
// Sets the cosmetic radius of the object
//
void Entity::SetOuterRadius(float newOuterRadius)
{
	m_outerRadius = newOuterRadius;
}


//-----------------------------------------------------------------------------------------------
// Returns the 2D position of the object
//
Vector2 Entity::GetPosition() const
{
	return m_position;
}


//-----------------------------------------------------------------------------------------------
// Returns the 2D velocity of the object
//
Vector2 Entity::GetVelocity() const
{
	return m_velocity;
}


//-----------------------------------------------------------------------------------------------
// Returns the rotation orientation of the object, in degrees
//
float Entity::GetOrientationDegrees() const
{
	return m_orientationDegrees;
}


//-----------------------------------------------------------------------------------------------
// Returns the rotation velocity of the object
//
float Entity::GetAngularVelocity() const
{
	return m_angularVelocity;
}


//-----------------------------------------------------------------------------------------------
// Returns the physics radius of the object
//
float Entity::GetInnerRadius() const
{
	return m_innerRadius;
}


//-----------------------------------------------------------------------------------------------
// Returns the cosmetic radius of the object
//
float Entity::GetOuterRadius() const
{
	return m_outerRadius;
}


//-----------------------------------------------------------------------------------------------
// Checks if the object is moving offscreen and if so sets its position to wrap around
//
void Entity::CheckWrapAround(const AABB2& worldBounds)
{
	// Left/Right screen check
	if (m_position.x < (-m_outerRadius))
	{
		m_position.x = (worldBounds.maxs.x + m_outerRadius);
	} else if (m_position.x > (worldBounds.maxs.x + m_outerRadius))
	{
		m_position.x = (-m_outerRadius);
	}

	// Top/Bottom screen check
	if (m_position.y > (worldBounds.maxs.y + m_outerRadius))
	{
		m_position.y = (-m_outerRadius);
	} else if (m_position.y < (-m_outerRadius))
	{
		m_position.y = (worldBounds.maxs.y + m_outerRadius);
	}
}


//-----------------------------------------------------------------------------------------------
// Checks if the object is moving offscreen and if so sets its position to wrap around
//
void Entity::DrawDebugInfo() const
{
	// Draw inner radius in cyan
	g_theRenderer->SetDrawColor(Rgba::CYAN);
	g_theRenderer->DrawDottedCircle(m_position, m_innerRadius);

	// Draw outer radius in magenta
	g_theRenderer->SetDrawColor(Rgba::MAGENTA);
	g_theRenderer->DrawDottedCircle(m_position, m_outerRadius);

	// Draw the 1-second prediction vector in yellow
	g_theRenderer->SetDrawColor(Rgba::YELLOW);
	Vector2 predictionPosition = m_position + m_velocity;
	g_theRenderer->DrawLineColor(m_position, Rgba::YELLOW, predictionPosition, Rgba::YELLOW);

	// Important - set color back to white
	g_theRenderer->SetDrawColor(Rgba::WHITE);
}
