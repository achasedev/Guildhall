/************************************************************************/
/* Project: Incursion
/* File: PlayerTank.hpp
/* Author: Andrew Chase
/* Date: September 28th, 2017
/* Bugs: None
/* Description: Class to represent a PlayerTank character in game
/************************************************************************/
#pragma once
#include "Game/Entity.hpp"
#include "Engine/Audio/AudioSystem.hpp"

class PlayerTank : public Entity
{
public:
	//-----Public Methods-----

	PlayerTank();
	virtual ~PlayerTank() override;

	// Override Entity
	virtual void Update(float deltaTime) override;
	virtual void Render() const override;

	bool IsInGodMode() const;

	void PerformDeathAnimation() const;
	void Respawn();			// Resets the player to not be marked for death and have the default health
	void ToggleGodMode();	// Toggles the player from invincible to not invincible, and vice versa

	void ApplySpeedBoost(float speedMultiplier);	// Gives the player a speed boost for a small amount of time

	std::vector<Vector2> GetBreadCrumbs() const;		// Returns the list of breadcrumbs used for follow behavoir

private:
	//-----Private Methods-----

	void ProcessInputAndUpdate(float deltaTime);		// Checks for PlayerTank related input this frame
	void FireBullet();									// Fires a bullet out of the front of the turret if the bullet cooldown is done=
	void UpdateBreadCrumbs(float deltaTime);			// Decrements the timer and adds a breadcrumb if the timer is up

private:
	//-----Private Data-----

	float m_turretOrientation;						// Degree orientation of the turret
	float m_bulletCooldown;							// Time until the next bullet can be fired
	bool m_inGodMode;								// Is the player currently invincible?
	float m_moveSpeed;								// Number of tiles the player can move per second
	float m_speedTimer;								// Track how long the player can move fast
	bool m_isMoving;
	SoundPlaybackID m_engineSoundPlayback;

	std::vector<Vector2> m_breadcrumbs;			// For ally follow behavior
	float m_breadCrumbTimer;

	static const int DEFAULT_PLAYER_HEALTH;			// Default health a player starts at
	static const float PLAYER_INNER_RADIUS;			// Physics radius of a player
	static const float PLAYER_OUTER_RADIUS;			// Cosmetic radius of a player
	static const float PLAYER_BULLETS_PER_SECOND; 	// Fire rate of the player
	static const float PLAYER_TURRET_TURN_SPEED;	// Degrees per second the player's turret turns
	static const float PLAYER_TANK_TURN_SPEED;		// Degrees per second the player's tank base turns
	static const float PLAYER_TANK_DEFAULT_MOVE_SPEED;		// Speed that the player moves forward, in tiles per second

	static const float PLAYER_INVINCIBILITY_DURATION;
	static const int PLAYER_INVINCIBILITY_FLASHES_PER_SECOND;
	static const float NUM_BREADCRUMBS_PER_SECOND;
};
