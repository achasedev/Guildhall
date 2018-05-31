/************************************************************************/
/* Project: SD1 C27 A1 - Asteroids
/* File: Game.cpp
/* Author: Andrew Chase
/* Date: August 30th, 2017
/* Bugs: None
/* Description: Implementation of the Game class, used for managing and 
/*				updating game objects and mechanics
/************************************************************************/
#include "Game.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Math/Disc2.hpp"



//-----------------------------------------------------------------------------------------------
// Finds an empty index in the asteroids array (if one exists) and sets all the values to
// appropriate ones specified in the documentation. 
// Returns true if an asteroid was initialized successfully, false otherwise
//
void Game::SpawnAsteroid(AsteroidSize newAsteroidSize)
{
	int index = FindEmptyAsteroidIndex();

	// if an index was found, spawn
	if (index != -1)
	{
		// Otherwise initialize the asteroid and return true
		m_asteroids[index] = new Asteroid(newAsteroidSize);
	}
}


//-----------------------------------------------------------------------------------------------
// Searches for a valid asteroid in the asteroid array in linear order, and if one is found sets
// it as invalid so it won't be drawn in the world, "destroying" it
// Returns true if an asteroid was set to invalid, false otherwise
//
bool Game::DestroyRandomAsteroid()
{
	// Search for an asteroid to delete
	for (int i = 0; i < MAX_ASTEROIDS; i++)
	{
		if (m_asteroids[i] != nullptr)
		{
			// if found, invalidate it and return true
			delete m_asteroids[i];
			m_asteroids[i] = nullptr;
			return true;
		}
	}

	// No valid asteroid exists in the list, so just return false
	return false;
}


//-----------------------------------------------------------------------------------------------
// Finds an empty (nullptr) element in the asteroid array and returns it's index
//
int Game::FindEmptyAsteroidIndex() const
{
	// Find an unused index in the asteroids array
	int index = -1;
	for (int i = 0; i < MAX_ASTEROIDS; i++)
	{
		if (m_asteroids[i] == nullptr)
		{
			index = i;
			break;
		}
	}

	return index;
}


//-----------------------------------------------------------------------------------------------
// Finds an empty (nullptr) element in the bullet array and returns it's index
//
int Game::FindEmptyBulletIndex() const
{
	int newIndex = -1;

	// Find an empty index for a bullet
	for (int i = 0; i < MAX_BULLETS; i++)
	{
		if (m_bullets[i] == nullptr)
		{
			newIndex = i;
			break;
		}
	}

	return newIndex;
}


//-----------------------------------------------------------------------------------------------
// Finds an empty index in the missile array and returns the index
//
int Game::FindEmptyMissileIndex() const
{
	int newIndex = -1;

	// Find an empty index for a bullet
	for (int i = 0; i < MAX_MISSILES; i++)
	{
		if (m_missiles[i] == nullptr)
		{
			newIndex = i;
			break;
		}
	}

	return newIndex;
}


//-----------------------------------------------------------------------------------------------
// Chooses the first asteroid in the asteroids list and returns it as a target
//
Asteroid* Game::FindMissileTarget()
{
	for (int i = 0; i < MAX_ASTEROIDS; i++)
	{
		if (m_asteroids[i] != nullptr)
		{
			return m_asteroids[i];
		}
	}

	return nullptr;
}

//-----------------------------------------------------------------------------------------------
// Spawns a new wave of m_numInitialAsteroids asteroids, and 'm_numInitialBlackholes' blackholes
//
void Game::StartNewWave()
{
	// Iterate through all asteroid indices
	for (int i = 0; i < MAX_ASTEROIDS; i++)
	{
		// For the first 'm_numInitialAsteroids' indices, create an asteroid
		if (i < m_numInitialAsteroids) {
			m_asteroids[i] = new Asteroid();

			// The rest invalidate
		} else {
			m_asteroids[i] = nullptr;
		}
	}

	// Spawn another black hole
	for (int i = 0; i < MAX_BLACKHOLES; i++)
	{
		// For the first 'm_numInitialBlackholes' indices, create an asteroid
		if (m_blackholes[i] == nullptr) 
		{
			m_blackholes[i] = new Blackhole();
			break;
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Starts the game in the initial play state as specified in the documentation - four to six asteroids
// are spawned with random values. All other available indices in the asteroids array are invalidated
// to prevent drawing uninitialized asteroids
//
void Game::SetupInitialGameState()
{
	// Start the game with a random number of asteroids
	m_numInitialAsteroids = GetRandomIntInRange(4, 6);

	StartNewWave();
	
	// Spawn the player ship
	m_playerShip = new Ship();
}


//-----------------------------------------------------------------------------------------------
// Checks if there are any asteroids left in the scene. If not, returns true, otherwise returns
// false
//
bool Game::AtEndOfWave() const
{
	// For all asteroids
	for (int i = 0; i < MAX_ASTEROIDS; i++)
	{
		// if it is valid, update it
		if (m_asteroids[i] != nullptr)
		{
			return false;
		}
	}

	return true;
}


//-----------------------------------------------------------------------------------------------
// Calls update on all asteroids
//
void Game::UpdateAsteroids(float deltaTime)
{
	// For all asteroids
	for (int i = 0; i < MAX_ASTEROIDS; i++)
	{
		// if it is valid, update it
		if (m_asteroids[i] != nullptr)
		{
			m_asteroids[i]->Update(deltaTime);
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Calls Update on the ship(s)
//
void Game::UpdateShip(float deltaTime)
{
	m_playerShip->Update(deltaTime);
}


//-----------------------------------------------------------------------------------------------
// Calls Update() on each bullet that isn't expired, and deletes each bullet that is expired
//
void Game::UpdateBullets(float deltaTime)
{
	for (int i = 0; i < MAX_BULLETS; i++)
	{
		Bullet* currBullet = m_bullets[i];
		if (currBullet != nullptr)
		{
			if (!currBullet->IsExpired())
			{
				currBullet->Update(deltaTime);
			} else {
   				delete currBullet;
				m_bullets[i] = nullptr;
			}
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Calls update on all missiles in the world
//
void Game::UpdateMissiles(float deltaTime)
{
	Asteroid* newTarget = FindMissileTarget();

	for (int i = 0; i < MAX_MISSILES; i++)
	{
		if (m_missiles[i] != nullptr)
		{
			// Ensures the missile has a target this frame
			m_missiles[i]->SetTarget(newTarget);
			m_missiles[i]->Update(deltaTime);
		}
	}

	// Also update the missile cooldown timer
	if (m_missileCooldown > 0.f)
	{
		m_missileCooldown -= deltaTime;
	}

	if (m_missileCooldown < 0.f)
	{
		m_missileCooldown = 0.f;
	}
}

//-----------------------------------------------------------------------------------------------
// Calls update on all blackholes in the world
//
void Game::UpdateBlackholes(float deltaTime)
{
	for (int i = 0; i < MAX_BLACKHOLES; i++)
	{
		if (m_blackholes[i] != nullptr)
		{
			m_blackholes[i]->Update(deltaTime);
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Checks each entity to ensure it wraps around the edge of the screen
//
void Game::CheckWrapAroundOnEntities()
{
	// Player ship
	m_playerShip->CheckWrapAround(WORLD_BOUNDS);

	// Asteroids
	for (int i = 0; i < MAX_ASTEROIDS; i++)
	{
		if (m_asteroids[i] != nullptr)
		{
			m_asteroids[i]->CheckWrapAround(WORLD_BOUNDS);
		}
	}

	// Bullets
	for (int i = 0; i < MAX_BULLETS; i++)
	{
		if (m_bullets[i] != nullptr)
		{
			m_bullets[i]->CheckWrapAround(WORLD_BOUNDS);
		}
	}

	// Black holes
	for (int i = 0; i < MAX_BLACKHOLES; i++)
	{
		if (m_blackholes[i] != nullptr)
		{
			m_blackholes[i]->CheckWrapAround(WORLD_BOUNDS);
		}
	}

	// Don't wrap around missiles, they track better if they can
	// turn around
}


//-----------------------------------------------------------------------------------------------
// Called when an asteroid is damaged, will either break the asteroid into two smaller asteroids
// or deletes the asteroid if it is already small
//
void Game::BreakAsteroid(int asteroidIndex)
{
	// Store the current asteroid information needed so we can delete it, freeing up it's space
	// in the array
	AsteroidSize currAsteroidSize = m_asteroids[asteroidIndex]->GetAsteroidSize();
	Vector2 currAsteroidPosition = m_asteroids[asteroidIndex]->GetPosition();

	delete m_asteroids[asteroidIndex];
	m_asteroids[asteroidIndex] = nullptr;

	if (currAsteroidSize != ASTEROIDSIZE_SMALL)
	{
		AsteroidSize newAsteroidSize = static_cast<AsteroidSize>(currAsteroidSize - 1);

		// Spawn first asteroid
		int firstIndex = FindEmptyAsteroidIndex();
		if (firstIndex != -1)
		{
			m_asteroids[firstIndex] = new Asteroid(newAsteroidSize);
			m_asteroids[firstIndex]->SetPosition(currAsteroidPosition);
		}

		// Spawn second asteroid
		int secondIndex = FindEmptyAsteroidIndex();
		if (secondIndex != -1)
		{
			m_asteroids[secondIndex] = new Asteroid(newAsteroidSize);
			m_asteroids[secondIndex]->SetPosition(currAsteroidPosition);
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Checks to see if the ship collided with any asteroids this frame. If a collision occurs,
// the ship is not rendered and the asteroid is deleted.
//
void Game::DetectShipAsteroidCollisions()
{
	for (int i = 0; i < MAX_ASTEROIDS; i++)
	{
		if (m_asteroids[i] == nullptr)
		{
			continue;
		}

		Asteroid* currAsteroid = m_asteroids[i];

		// Get values now for organization
		Vector2 shipPosition = m_playerShip->GetPosition();
		float shipRadius = m_playerShip->GetInnerRadius();
		Vector2 asteroidPosition = currAsteroid->GetPosition();
		float asteroidRadius = currAsteroid->GetInnerRadius();

		if (DoDiscsOverlap(shipPosition, shipRadius, asteroidPosition, asteroidRadius))
		{
			// There is a collision, so destroy the asteroid and derender the ship
			m_playerShip->SetIsDead(true);
			m_screenShakeTimer = SHAKE_DURATION;
			BreakAsteroid(i);
		}
		
	}
}


//-----------------------------------------------------------------------------------------------
// Checks to see if any bullets collided with any asteroids this frame. If a collision occurs,
// both the bullet and the asteroid are deleted.
//
void Game::DetectBulletAsteroidCollisions()
{
	for (int i = 0; i < MAX_ASTEROIDS; i++)
	{
		if (m_asteroids[i] == nullptr)
		{
			continue;
		}

		Asteroid* currAsteroid = m_asteroids[i];

		for (int j = 0; j < MAX_BULLETS; j++)
		{
			if (m_bullets[j] == nullptr)
			{
				continue;
			}

			Bullet* currBullet = m_bullets[j];

			// Get values now for organization
			Vector2 asteroidPosition = currAsteroid->GetPosition();
			float asteroidRadius = currAsteroid->GetInnerRadius();
			Vector2 bulletPosition = currBullet->GetPosition();
			float bulletRadius = currBullet->GetInnerRadius();

			if (DoDiscsOverlap(asteroidPosition, asteroidRadius, bulletPosition, bulletRadius))
			{
				// There is a collision, so break/destroy the asteroid and destroy bullet
				BreakAsteroid(i);

				delete m_bullets[j];
				m_bullets[j] = nullptr;

				break;
			}
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Checks all entities to see if they overlap the Black hole, and if so adjusts their velocity
// to move towards the black hole
//
void Game::DetectMissileAsteroidCollisions()
{
	for (int i = 0; i < MAX_ASTEROIDS; i++)
	{
		if (m_asteroids[i] == nullptr)
		{
			continue;
		}

		Asteroid* currAsteroid = m_asteroids[i];

		for (int j = 0; j < MAX_MISSILES; j++)
		{
			if (m_missiles[j] == nullptr)
			{
				continue;
			}

			Missile* currMissile = m_missiles[j];

			// Get values now for organization
			Vector2 asteroidPosition = currAsteroid->GetPosition();
			float asteroidRadius = currAsteroid->GetInnerRadius();
			Vector2 missilePosition = currMissile->GetPosition();
			float missileRadius = currMissile->GetInnerRadius();

			if (DoDiscsOverlap(asteroidPosition, asteroidRadius, missilePosition, missileRadius))
			{
				// There is a collision, so destroy both entities
				delete m_asteroids[i];
				m_asteroids[i] = nullptr;

				delete m_missiles[j];
				m_missiles[j] = nullptr;

				// Apply a screen shake for effect
				m_screenShakeTimer = SHAKE_DURATION;
				break;
			}
		}
	}
}

//-----------------------------------------------------------------------------------------------
// Checks all entities to see if they overlap the Black hole, and if so adjusts their velocity
// to move towards the black hole
//
void Game::ApplyBlackholeGravities(float deltaTime)
{
	for (int i = 0; i < MAX_BLACKHOLES; i++)
	{

		Blackhole* currBlackhole = m_blackholes[i];
		if (currBlackhole == nullptr)
		{
			continue;
		}

		// Localize values for organization
		Vector2 blackHolePosition = currBlackhole->GetPosition();
		float blackHoleRadius = currBlackhole->GetGravityRadius();

		// Check against all asteroids
		for (int j = 0; j < MAX_ASTEROIDS; j++)
		{
			Asteroid* currAsteroid = m_asteroids[j];
			if (currAsteroid == nullptr)
			{
				continue;
			}

			Vector2 asteroidPosition = currAsteroid->GetPosition();
			float asteroidRadius = currAsteroid->GetInnerRadius();
			if (DoDiscsOverlap(blackHolePosition, blackHoleRadius, asteroidPosition, asteroidRadius))
			{
				Vector2 force = (currBlackhole->GetPosition() - currAsteroid->GetPosition()).GetNormalized();
				currAsteroid->SetVelocity(currAsteroid->GetVelocity() + (Blackhole::GRAVITY_SCALAR * force * deltaTime));
			}
		}

		// Check against all bullets
		for (int j = 0; j < MAX_BULLETS; j++)
		{
			Bullet* currBullet = m_bullets[j];
			if (currBullet == nullptr)
			{
				continue;
			}

			Vector2 bulletPosition = currBullet->GetPosition();
			float bulletRadius = currBullet->GetInnerRadius();

			if (DoDiscsOverlap(blackHolePosition, blackHoleRadius, bulletPosition, bulletRadius))
			{
				Vector2 force = (currBlackhole->GetPosition() - currBullet->GetPosition()).GetNormalized();
				currBullet->SetVelocity(currBullet->GetVelocity() + (Blackhole::GRAVITY_SCALAR * force * deltaTime));
			}
		}

		// Check against all missiles
		for (int j = 0; j < MAX_MISSILES; j++)
		{
			Missile* currMissile = m_missiles[i];
			if (currMissile == nullptr)
			{
				continue;
			}

			Vector2 missilePosition = currMissile->GetPosition();
			float missileRadius = currMissile->GetInnerRadius();

			if (DoDiscsOverlap(blackHolePosition, blackHoleRadius, missilePosition, missileRadius))
			{
				Vector2 force = (currBlackhole->GetPosition() - currMissile->GetPosition()).GetNormalized();
				currMissile->SetVelocity(currMissile->GetVelocity() + (Blackhole::GRAVITY_SCALAR * force * deltaTime));
			}
		}

		// Check against the player ship, if it is not dead or invincible
		if (!m_playerShip->IsDead() && !m_playerShip->IsInvincible())
		{
			Vector2 shipPosition = m_playerShip->GetPosition();
			float shipRadius = m_playerShip->GetInnerRadius();

			if (DoDiscsOverlap(blackHolePosition, blackHoleRadius, shipPosition, shipRadius))
			{
				Vector2 force = (currBlackhole->GetPosition() - m_playerShip->GetPosition()).GetNormalized();
				m_playerShip->SetVelocity(m_playerShip->GetVelocity() + (Blackhole::GRAVITY_SCALAR * force * deltaTime));
			}
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Checks if there is currently screen shake, and if there is calls glOrtho given the world bounds
// and a new randomized offset
//
void Game::UpdateScreenShake(float deltaTime)
{
	if (m_screenShakeTimer > 0)
	{
		// Adjust coordinate system for screenshake
		AABB2 drawingbounds = WORLD_BOUNDS;
		if (m_screenShakeTimer > 0)
		{
			float shakeOffsetX = GetRandomFloatInRange(0.f, SHAKE_MAGNITUDE * m_screenShakeTimer);
			float shakeOffsetY = GetRandomFloatInRange(0.f, SHAKE_MAGNITUDE * m_screenShakeTimer);

			drawingbounds.Translate(Vector2(shakeOffsetX, shakeOffsetY));
		}
		g_theRenderer->SetOrtho(drawingbounds);
		m_screenShakeTimer -= deltaTime;
	}
}


//-----------------------------------------------------------------------------------------------
// Creates a new black hole in the game, if there is room for one
//
void Game::SpawnBlackhole()
{
	for (int i = 0; i < MAX_BLACKHOLES; i++)
	{
		if (m_blackholes[i] == nullptr)
		{
			m_blackholes[i] = new Blackhole();
			return;
		}
	}
}

//-----------------------------------------------------------------------------------------------
// Updates the game state by processing input received this frame (respawning player, firing a bullet).
//
void Game::ProcessInputThisFrame()
{
	XboxController& theController = g_theInputSystem->GetController(0);
	bool useControllerInput = false;

	// Firing a bullet
	bool aJustPressed = theController.WasButtonJustPressed(XBOX_BUTTON_A);
	useControllerInput = (theController.IsConnected() && aJustPressed);
	if ((g_theInputSystem->WasKeyJustPressed(' ') || useControllerInput) && !m_playerShip->IsDead())
	{
		FireBulletOutOfShip();
	}

	// Respawning a player
	bool startJustPressed = theController.WasButtonJustPressed(XBOX_BUTTON_START);
	useControllerInput = (theController.IsConnected() && startJustPressed);
	if ((g_theInputSystem->WasKeyJustPressed('N') || useControllerInput) && m_playerShip->IsDead())
	{
		m_playerShip->Respawn();
	}

	// Spawning an asteroid (default size is large)
	if (g_theInputSystem->WasKeyJustPressed('I'))
	{
		SpawnAsteroid(ASTEROIDSIZE_LARGE);
	}

	// Destroying a random asteroid
	if (g_theInputSystem->WasKeyJustPressed('O'))
	{
		DestroyRandomAsteroid();
	}

	// Spawn a missile
	bool xJustPressed = theController.WasButtonJustPressed(XBOX_BUTTON_X);
	useControllerInput = (theController.IsConnected() && xJustPressed);
	if((g_theInputSystem->WasKeyJustPressed('M') || useControllerInput) && (m_missileCooldown == 0.f))
	{
		FireMissileOutOfShip();
		m_missileCooldown = 1.f;
	}

	// Spawn a black hole
	if (g_theInputSystem->WasKeyJustPressed('B'))
	{
		SpawnBlackhole();
	}

	// Drawing Debug information
	if (g_theInputSystem->WasKeyJustPressed(InputSystem::KEYBOARD_F1))
	{
		m_inDebugMode = !m_inDebugMode;
	}
}


//-----------------------------------------------------------------------------------------------
// Update the movement variables of every entity in the world, as well as update the game state
// based on the input this frame
//
void Game::Update(float deltaTime)
{
	// Process game-related input
	ProcessInputThisFrame();

	// Check wave first, to ensure asteroids are always
	// present (so missiles always have a target)
	if (AtEndOfWave())
	{
		m_numInitialAsteroids += 2;
		StartNewWave();
	}

	// Update the screen shake effect, if applicable
	UpdateScreenShake(deltaTime);

	// Call update on all entities
	UpdateAsteroids(deltaTime);
	UpdateShip(deltaTime);
	UpdateBullets(deltaTime);
	UpdateMissiles(deltaTime);
	UpdateBlackholes(deltaTime);

	// Wrap around entities that moved off screen
	CheckWrapAroundOnEntities();

	// Do collisions last, to avoid asteroids not existing
	// During this frame (ensures missiles always have a target)
	DetectBulletAsteroidCollisions();
	DetectMissileAsteroidCollisions();

	if (!m_playerShip->IsDead() && !m_playerShip->IsInvincible())
	{
		DetectShipAsteroidCollisions();
	}

	// Apply a force to all entities near a black hole
	ApplyBlackholeGravities(deltaTime);
}


//-----------------------------------------------------------------------------------------------
// Draws all asteroids currently in the game
//
void Game::DrawAsteroids() const
{
	// For each asteroid
	for (int i = 0; i < MAX_ASTEROIDS; i++)
	{
		Asteroid* currAsteroid = m_asteroids[i];
		// If it is valid, draw it
		if (currAsteroid != nullptr)
		{
			currAsteroid->Render();
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Draws the player ship
//
void Game::DrawShip() const
{
	m_playerShip->Render();
}


//-----------------------------------------------------------------------------------------------
// Draws all non-expired bullets in the game
//
void Game::DrawBullets() const
{
	for (int i = 0; i < MAX_BULLETS; i++)
	{
		if (m_bullets[i] != nullptr && !m_bullets[i]->IsExpired())
		{
			m_bullets[i]->Render();
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Calls Render on all Missile objects in the scene
//
void Game::DrawMissiles() const
{
	for (int i = 0; i < MAX_MISSILES; i++)
	{
		if (m_missiles[i] != nullptr)
		{
			m_missiles[i]->Render();
		}
	}
}

//-----------------------------------------------------------------------------------------------
// Calls Render on all Black hole objects in the scene
//
void Game::DrawBlackholes() const
{
	for (int i = 0; i < MAX_BLACKHOLES; i++)
	{
		if (m_blackholes[i] != nullptr)
		{
			m_blackholes[i]->Render();
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Sets up the coordinate system and draws all Entities
//
void Game::DrawDebugInformation() const
{
	// Draw asteroid information
	for (int i = 0; i < MAX_ASTEROIDS; i++)
	{
		if (m_asteroids[i] != nullptr)
		{
			m_asteroids[i]->DrawDebugInfo();
		}
	}

	// Draw ship information
	m_playerShip->DrawDebugInfo();

	// Draw bullet information
	for (int i = 0; i < MAX_BULLETS; i++)
	{
		if (m_bullets[i] != nullptr)
		{
			m_bullets[i]->DrawDebugInfo();
		}
	}

	// Draw missile information
	for (int i = 0; i < MAX_MISSILES; i++)
	{
		if (m_missiles[i] != nullptr)
		{
			m_missiles[i]->DrawDebugInfo();
		}
	}

	// Draw Black hole information
	for (int i = 0; i < MAX_BLACKHOLES; i++)
	{
		if (m_blackholes[i] != nullptr)
		{
			m_blackholes[i]->DrawDebugInfo();
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Sets up the coordinate system and draws all Entities
//
void Game::Render() const
{
	// Draw all of the asteroids in the game this frame
	DrawAsteroids();
	// Draw the player ship, if it's not dead
	if (!m_playerShip->IsDead())
	{
		DrawShip();
	}
	// Draw the bullets
	DrawBullets();

	// Draw the Missiles
	DrawMissiles();

	// Draw the Blackholes
	DrawBlackholes();

	// Draw debug information if in debug mode
	if (m_inDebugMode)
	{
		DrawDebugInformation();
	}
}


//-----------------------------------------------------------------------------------------------
// Spawns a bullet out of the ship's nose
//
void Game::FireBulletOutOfShip()
{
	if (!m_playerShip->IsDead())
	{
		// Find an empty index in the bullet array
		int newIndex = FindEmptyBulletIndex();

		// No empty index (64 bullets in world already), so do nothing
		if (newIndex == -1)
		{
			return;
		}

		// Calculate the bullet's initial position and fire direction, and then create it
		Vector2 forwardDirection = Vector2::MakeDirectionAtDegrees(m_playerShip->GetOrientationDegrees());
		Vector2 noseOffset = forwardDirection * m_playerShip->GetOuterRadius();
		Vector2 nosePosition = m_playerShip->GetPosition() + noseOffset;

		Vector2 fireDirection = (nosePosition - m_playerShip->GetPosition()).GetNormalized();

		m_bullets[newIndex] = new Bullet(nosePosition, fireDirection, m_playerShip->GetVelocity(), m_playerShip->GetOrientationDegrees());	
	}
}


//-----------------------------------------------------------------------------------------------
// Fires a missile out of the nose of the ship, and applies recoil
//
void Game::FireMissileOutOfShip()
{
	if (!m_playerShip->IsDead())
	{
		// Find an empty index in the bullet array
		int newIndex = FindEmptyMissileIndex();

		// No empty index (32 missiles in world already), so do nothing
		if (newIndex == -1)
		{
			return;
		}

		// Calculate the missile's initial position and fire direction, and then create it
		Vector2 forwardDirection = Vector2::MakeDirectionAtDegrees(m_playerShip->GetOrientationDegrees());
		Vector2 noseOffset = forwardDirection * m_playerShip->GetOuterRadius();
		Vector2 nosePosition = m_playerShip->GetPosition() + noseOffset;

		Vector2 fireDirection = (nosePosition - m_playerShip->GetPosition()).GetNormalized();

		// Get the target for the missile
		Asteroid* missileTarget = FindMissileTarget();

		m_missiles[newIndex] = new Missile(nosePosition, fireDirection, m_playerShip->GetOrientationDegrees(), missileTarget);	
	}

	// Apply the recoil from the missile firing
	m_playerShip->ApplyRecoil();
}

//-----------------------------------------------------------------------------------------------
// Toggles whether or not the debug information should be drawn
//
void Game::ToggleDebugMode()
{
	m_inDebugMode = !m_inDebugMode;
}
