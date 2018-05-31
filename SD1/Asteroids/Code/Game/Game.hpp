/************************************************************************/
/* Project: SD1 C27 A1 - Asteroids
/* File: Game.hpp
/* Author: Andrew Chase
/* Date: August 30th, 2017
/* Bugs: None
/* Description: Class used for managing and updating game objects and 
/*              mechanics
/************************************************************************/
#pragma once
#include "Game/Asteroid.hpp"
#include "Game/Ship.hpp"
#include "Game/Bullet.hpp"
#include "Game/Blackhole.hpp"
#include "Game/Missile.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/AABB2.hpp"


class Game
{
	
public:

	//-----Public Methods-----
	void SetupInitialGameState();		// Clears the game's asteroid list and initialized four asteroids


	void Update(float deltaTime);		// Updates all game object states, called each frame
	void Render() const;				// Renders all game objects to screen, called each frame
	void ToggleDebugMode();				// Sets the Game flag to render Debug information


private:

	//-----Private Methods-----
	bool AtEndOfWave() const;			// Checks if there are no asteroids in the scene

	// Updating
	void UpdateAsteroids(float deltaTime);				
	void UpdateShip(float deltaTime);					
	void UpdateBullets(float deltaTime);	
	void UpdateMissiles(float deltaTime);
	void UpdateBlackholes(float deltaTime);				
	void UpdateScreenShake(float deltaTime);			
	void ProcessInputThisFrame();						
	
	void CheckWrapAroundOnEntities();				// Checks all Entities to ensure they wrap around the screen
	void DetectShipAsteroidCollisions();				
	void DetectBulletAsteroidCollisions();	
	void DetectMissileAsteroidCollisions();
	void ApplyBlackholeGravities(float deltaTime);	// Accelerates all non-blackhole entities to a black hole, if in range

	// Rendering
	void DrawAsteroids() const;							
	void DrawShip() const;								
	void DrawBullets() const;	
	void DrawMissiles() const;
	void DrawBlackholes() const;						

	// Utility
	void DrawDebugInformation() const;		// Draws the inner radii, outer radii, and 1-second position predictions for all Entities
	void BreakAsteroid(int asteroidIndex);	// Called when an asteroid is shot, either breaks it into 2 smaller asteroids or deletes it
	void StartNewWave();					// Spawns a new wave of Asteroids and Blackholes
							
	void SpawnAsteroid(AsteroidSize newAsteroidSize);	
	void FireBulletOutOfShip();				// Spawns a bullet out of the ship's nose
	void FireMissileOutOfShip();
	void SpawnBlackhole();	
	bool DestroyRandomAsteroid();								

	int FindEmptyAsteroidIndex() const;		// Finds an empty (nullptr) element in the asteroids array
	int FindEmptyBulletIndex() const;		// Finds an empty (nullptr) element in the bullets array
	int FindEmptyMissileIndex() const;		// Finds an empty (nullptr) element in the missiles array
	Asteroid* FindMissileTarget();			// Returns a random Asteroid for the missile to target

private:

	//-----Private Data-----
	Asteroid* m_asteroids[64];											// List for managing active asteroids
	Ship* m_playerShip;													// The player ship
	Bullet* m_bullets[64];												// List of bullets fired from the ship
	Missile* m_missiles[32];
	Blackhole* m_blackholes[8];											// List of Blackholes that exist in the Game

	bool m_inDebugMode = false;											// Indicates whether debug information is rendered or not
	float m_screenShakeTimer = 0.f;										// Timer to indicate how much longer to screenshake
	float m_missileCooldown = 0.f;
	int m_numInitialAsteroids;											// Number of asteroids to spawn during the next wave

	const AABB2 WORLD_BOUNDS = AABB2(0.f, 0.f, 1000.f, 1000.f);			// Bounds of the draw space
	static const int MAX_ASTEROIDS = 64;								// Maximum number of asteroids that can exist at one time
	static const int MAX_BULLETS = 64;									// Maximum number of bullets that can exist at one time
	static const int MAX_MISSILES = 32;
	static const int MAX_BLACKHOLES = 8;

	static constexpr float SHAKE_DURATION = 1.f;						// How long to screen shake for
	static constexpr float SHAKE_MAGNITUDE = 50.f;						// Offset that the screen shake vibrates
};
