/************************************************************************/
/* Project: SD1 C27 A2 - Asteroids
/* File: Ship.hpp
/* Author: Andrew Chase
/* Date: September 8th, 2017
/* Bugs: None
/* Description: Class to represent the player-controlled ship
/************************************************************************/
#pragma once
#include "Game/Entity.hpp"


class Ship : public Entity
{
public:

	Ship();

	//-----Public Methods-----
	void Update(float deltaTime);						// For moving the ship
	void Render() const;								// For drawing the ship

	bool IsDead() const;								// Returns whether or not the ship is dead
	bool IsInvincible() const;							// Returns true if the player is invincible
	void SetIsDead(bool newState);						// Marks the ship as being destroyed

	void Respawn();										// Resets the ship back to its initial game state
	void ApplyRecoil();									// Applies the gun recoil in the opposite direction the ship is facing

private:

	void UpdateLinearVelocity(float deltaTime);
	void UpdateAngularVelocity();
	void UpdateInvincibilityTimer(float deltaTime);

	//-----Private Data-----
	bool m_isDead;												// True if the player died
	float m_invincibilityTimer = 3.f;							// For spawning, make the player invincible for 3 seconds
	
	static constexpr float BASE_SHIP_ACCELERATION = 180.f;		// Rate at which the ship speeds up when 'Up' is pressed
	static constexpr float SHIP_TURN_SPEED = 300.f;				// Speed at which the ship turns
	static const int FLASHES_PER_SECOND = 4;					// When invincible, the number of times the ship flashes per second
};
