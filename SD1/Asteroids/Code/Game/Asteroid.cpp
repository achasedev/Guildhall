/************************************************************************/
/* Project: SD1 C27 A1 - Asteroids
/* File: Asteroid.cpp
/* Author: Andrew Chase
/* Date: August 30th, 2017
/* Bugs: None
/* Description: Handles initializing and updating asteroid values
/************************************************************************/
#include "Game/Asteroid.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"


//-----Static Constants-----
const float Asteroid::ASTEROID_SPEED = 150.f;		// Speed that all asteroids should translate by
const int Asteroid::ASTEROID_NUM_SIDES = 30;		// Number of sides that all asteroids will have
const float Asteroid::SMALL_INNER_RADIUS = 10.f;	// The inner radius of a small asteroid
const float Asteroid::SMALL_OUTER_RADIUS = 12.5f;	// The outer radius of a small asteroid


//-----------------------------------------------------------------------------------------------
// Default Constructor - used to initialize all inherited members from Entity as well as 
// set the radius of the asteroid to the large setting
//
Asteroid::Asteroid()
{
	InitializeValues(ASTEROIDSIZE_LARGE);	
}


//-----------------------------------------------------------------------------------------------
// Constructor to create an asteroid of size newAsteroidSize
//
Asteroid::Asteroid(AsteroidSize newAsteroidSize)
{
	InitializeValues(newAsteroidSize);
}


//-----------------------------------------------------------------------------------------------
// Initializes the asteroid to have the default values (random location, velocity, etc.)
//
void Asteroid::InitializeValues(AsteroidSize newAsteroidSize)
{
	// Set position of the asteroid just outside the game bounds
	// Generate random true/false to decide whether to place it above or to
	// the right of the screen
	if (GetRandomTrueOrFalse())
	{
		m_position = Vector2(GetRandomFloatInRange(0.f, 1000.f), 1500.f);
	} 
	else
	{
		m_position = Vector2(1500.f, GetRandomFloatInRange(0.f, 1000.f));
	}

	// Orientation angle used for drawing, will change later over time
	m_orientationDegrees = 0.f;

	// Set rotation speed of this asteroid as a random positive degree value
	m_angularVelocity = GetRandomFloatInRange(30.f, 120.f);

	// Flip it negative randomly, to ensure a random direction with a decently large
	// magnitude
	if (GetRandomTrueOrFalse())
	{
		m_angularVelocity *= -1.f;
	}

	// For the size, generate a random int between 1 and 3 to decide whether the asteroid should be
	// small, medium, or large.
	m_asteroidSize = newAsteroidSize;

	if (m_asteroidSize == ASTEROIDSIZE_SMALL)
	{
		m_innerRadius = SMALL_INNER_RADIUS;
		m_outerRadius = SMALL_OUTER_RADIUS;
	} 
	else if (m_asteroidSize == ASTEROIDSIZE_MEDIUM) 
	{
		m_innerRadius = 2.f * SMALL_INNER_RADIUS;
		m_outerRadius = 2.f * SMALL_OUTER_RADIUS;
	} 
	else if (m_asteroidSize == ASTEROIDSIZE_LARGE)
	{
		m_innerRadius = 4.f * SMALL_INNER_RADIUS;
		m_outerRadius = 4.f * SMALL_OUTER_RADIUS;
	} else
	{
		// Debug case, although this shouldn't happen, make the asteroid abnormally large
		// to make it obvious
		m_innerRadius = 8.f * SMALL_INNER_RADIUS;
		m_outerRadius = 8.f * SMALL_OUTER_RADIUS;
	}

	// Direction the polygon will be moving...
	m_velocity = (Vector2(GetRandomFloatInRange(-100.f, 100.f), GetRandomFloatInRange(-100.f, 100.f)));
	// ...normalized...
	m_velocity = m_velocity.GetNormalized();
	// ...and scaled based on the asteroids' set speed
	m_velocity *= ASTEROID_SPEED;

	// Setup the vertex positions
	for (int i = 0; i < ASTEROID_NUM_SIDES; i++)
	{
		float degressPerSide = 360.f * (1.f / ASTEROID_NUM_SIDES);
		float currDegrees = degressPerSide * static_cast<float>(i);


		// Temporary floats used to plot vertices around the unit circle (distance from radius <= 1)
		float minRadius = m_innerRadius * (1.f / m_outerRadius);
		float maxRadius = 1.0f;

		// The farthest vertex here is on the outer radius, so the cosmetic radius will contain the asteroid
		float randomRadius = GetRandomFloatInRange(minRadius, maxRadius);

		m_vertexPositions[i] = Vector2(randomRadius * CosDegrees(currDegrees), randomRadius * SinDegrees(currDegrees));
	}
}


//-----------------------------------------------------------------------------------------------
// Updates the movement values (rotation and translation) of this asteroid so that it will move 
// according to the deltaTime of the last frame
//
void Asteroid::Update(float deltaTime)
{
	// Just call the Entity Update for position and orientation updating
	Entity::Update(deltaTime);
}


//-----------------------------------------------------------------------------------------------
// Draws the asteroid to screen in the world space
//
void Asteroid::Render() const
{
	// Setup coordinate system
	g_theRenderer->PushMatrix();
	g_theRenderer->TranslateCoordinateSystem(m_position.x, m_position.y, 0.f);
	g_theRenderer->RotateCoordinateSystem2D(m_orientationDegrees);
	g_theRenderer->ScaleCoordinateSystem(m_outerRadius);

	// Draw the lines
	g_theRenderer->SetDrawColor(Rgba::BROWN);
	for (int i = 0; i < ASTEROID_NUM_SIDES; i++)
	{
		g_theRenderer->DrawLine(m_vertexPositions[i], m_vertexPositions[(i + 1) % ASTEROID_NUM_SIDES]);
	}

	// Set the color back to white
	g_theRenderer->SetDrawColor(Rgba::WHITE);

	// Restore the coordinate system
	g_theRenderer->PopMatrix();
}


AsteroidSize Asteroid::GetAsteroidSize()
{
	return m_asteroidSize;
}
