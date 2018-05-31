/************************************************************************/
/* Project: SD1 C27 A1 - Asteroids
/* File: Asteroid.hpp
/* Author: Andrew Chase
/* Date: August 30th 2017
/* Bugs: None
/* Description: Class used to represent a single asteroid in the game
/************************************************************************/
#pragma once
#include "Game/Entity.hpp"
#include "Engine/Math/Vector2.hpp"


enum AsteroidSize
{
	ASTEROIDSIZE_UNKNOWN = -1,
	ASTEROIDSIZE_SMALL,
	ASTEROIDSIZE_MEDIUM,
	ASTEROIDSIZE_LARGE,
	NUM_ASTEROIDSIZES
};

//-----------------------------------------------------------------------------------------------
class Asteroid : public Entity
{
		
public:

	
	Asteroid();											// Default Constructor
	Asteroid(AsteroidSize newAsteroidSize);				// Constructor to create a new asteroid of size newAsteroidSize

	void InitializeValues(AsteroidSize asteroidSize);	// Sets all the Asteroid member variables to the default values with size asteroidSize

	//-----Public Methods-----
	void Update(float deltaTime);						// Move the asteroid in the world based on its movement values
	void Render() const;								// Draws the asteroid in the world	

	AsteroidSize GetAsteroidSize();

private:

	//-----Private Data-----
	Vector2 m_vertexPositions[30];				// Vertices for the asteroid's shape
	AsteroidSize m_asteroidSize;				// Size of the asteroid, between small, medium, and large

	//-----Private Static Members-----
	const static int ASTEROID_NUM_SIDES;		// Number of sides this asteroid has
	const static float ASTEROID_SPEED;			// Default translation speed of all asteroids
	const static float SMALL_INNER_RADIUS;		// The inner radius of a small asteroid
	const static float SMALL_OUTER_RADIUS;		// The outer radius of a small asteroid
};
