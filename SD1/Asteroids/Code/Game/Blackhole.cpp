/************************************************************************/
/* Project: SD1 C27 Asteroids Gold
/* File: Blackhole.cpp
/* Author: Andrew Chase
/* Date: September 19th, 2017
/* Bugs: None
/* Description: Implementation of the Blackhole class
/************************************************************************/
#include "Game/Blackhole.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"


//-----------------------------------------------------------------------------------------------
// Default constructor, is contructed much like an Asteroid
//
Blackhole::Blackhole()
{
	// Choose to spawn randomly above or to the right of the game bounds
	if (GetRandomTrueOrFalse())
	{
		m_position = Vector2(GetRandomFloatInRange(0.f, 1000.f), 1500.f);
	} 
	else
	{
		m_position = Vector2(1500.f, GetRandomFloatInRange(0.f, 1000.f));
	}

	// Setting inherited data to constant values
	m_velocity = (Vector2(GetRandomFloatInRange(-100.f, 100.f), GetRandomFloatInRange(-100.f, 100.f)));

	m_orientationDegrees = 0.f;
	m_angularVelocity = 180.f;

	m_innerRadius = 10.f;
	m_outerRadius = 150.f;

	m_age = 0;
	m_ageAtDeath = -1;

	// Black hole data member
	m_gravityRadius = 250.f;
}


//-----------------------------------------------------------------------------------------------
// Updates by calling the inherited Update from Entity
//
void Blackhole::Update(float deltaTime)
{
	Entity::Update(deltaTime);
}


//-----------------------------------------------------------------------------------------------
// Draws the black hole to screen
//
void Blackhole::Render() const
{
	// Set up coordinate system
	g_theRenderer->PushMatrix();
	g_theRenderer->TranslateCoordinateSystem(m_position.x, m_position.y, 0.f);
	g_theRenderer->RotateCoordinateSystem2D(m_orientationDegrees);
	g_theRenderer->ScaleCoordinateSystem(m_outerRadius);
	
	Rgba drawColor = Rgba::PURPLE;

	for (int i = 0; i < 255; i++)
	{
		// Start on the outside of the spiral, and slowly close in
		float radius = (1.f - static_cast<float>(i) * (1.f / 255.f));

		// Calculate start and end degree bounds for this side
		float degreesPerSide = 20.f;
		float startDegrees = (i * degreesPerSide);
		float endDegrees = (startDegrees + degreesPerSide);

		// calculate the start and end components for the two endpoints of this line
		float startX = radius * CosDegrees(startDegrees);
		float startY = radius * SinDegrees(startDegrees);
		float endX = radius * CosDegrees(endDegrees);
		float endY = radius * SinDegrees(endDegrees);

		// Calculate an alpha value that gets more opaque as we reach the center
		unsigned char alphaValue = static_cast<unsigned char>(RangeMapFloat(static_cast<float>(i), 0.f, 255.f, 40.f, 255.f));

		// Set the color
		drawColor.SetAsBytes(drawColor.r, drawColor.g, drawColor.b, alphaValue);
		g_theRenderer->SetDrawColor(drawColor);

		// Draw the line segment
		g_theRenderer->DrawLine(Vector2(startX, startY), Vector2(endX, endY));
	}

	// Set the color back to white
	g_theRenderer->SetDrawColor(Rgba::WHITE);

	// Reset the coordinate system
	g_theRenderer->PopMatrix();
}


//-----------------------------------------------------------------------------------------------
// Accessor for the gravity radius
//
float Blackhole::GetGravityRadius() const
{
	return m_gravityRadius;
}
