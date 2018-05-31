/************************************************************************/
/* Project: SD1 C27 A2 - Asteroids Playable
/* File: Bullet.hpp
/* Author: Andrew Chase
/* Date: September 9th, 2017
/* Bugs: None
/* Description: Class used to represent a bullet projectile in the game
/************************************************************************/
#pragma once
#include "Game/Entity.hpp"


class Bullet : public Entity
{
public:

	// Constructor - Spawns the bullet at the ship's nose, and sets the velocity to fire forward relative to the ship
	Bullet(Vector2 nosePosition, Vector2 fireDirection, Vector2 shipVelocity, float shipOrientation);	

	//-----Public Methods-----
	void Update(float deltaTime);		// Updates the bullet's position and decrements its time to live each frame
	void Render() const;				// Draws the bullet to screen

	bool IsExpired() const;				// Returns true if the ship has been spawned and in game for two seconds

private:

	//-----Private Data-----
	float m_timeToLive;					// Returns true if the bullet has been spawned for 2 seconds

	//-----Private Static Members-----
	static const int BULLET_NUM_SIDES;	// High value, so the Bullet appears circular
	static const float BULLET_SPEED;	// Default speed of all Bullets
};
