/************************************************************************/
/* Project: Incursion
/* File: PlayerTank.cpp
/* Author: Andrew Chase
/* Date: September 28th, 2017
/* Bugs: None
/* Description: Implementation of the PlayerTank class
/************************************************************************/
#include "Game/Game.hpp"
#include "Game/PlayerTank.hpp"
#include "GameCommon.hpp"
#include "Game/Bullet.hpp"
#include "Game/Map.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"

// Static Constants
const float PlayerTank::PLAYER_INNER_RADIUS							= 0.35f;
const float PlayerTank::PLAYER_OUTER_RADIUS							= 0.5f;
const float PlayerTank::PLAYER_BULLETS_PER_SECOND					= 10.f;
const int	PlayerTank::DEFAULT_PLAYER_HEALTH						= 10;
const float	PlayerTank::PLAYER_TANK_TURN_SPEED						= 180.f;
const float PlayerTank::PLAYER_TURRET_TURN_SPEED					= 360.f;
const float PlayerTank::PLAYER_TANK_DEFAULT_MOVE_SPEED				= 2.f;

const float PlayerTank::PLAYER_INVINCIBILITY_DURATION				= 3.f;
const int	PlayerTank::PLAYER_INVINCIBILITY_FLASHES_PER_SECOND		= 4;
const float PlayerTank::NUM_BREADCRUMBS_PER_SECOND					= 4.0f;


//-----------------------------------------------------------------------------------------------
// Constructor - currently just places the player in the center of the screen, facing right
//
PlayerTank::PlayerTank()
	: m_turretOrientation(0)
	, m_bulletCooldown(1.f / PLAYER_BULLETS_PER_SECOND)
	, m_inGodMode(false)
	, m_isMoving(false)
	, m_breadCrumbTimer(1.f / NUM_BREADCRUMBS_PER_SECOND)
	, m_moveSpeed(PLAYER_TANK_DEFAULT_MOVE_SPEED)
	, m_speedTimer(0.f)
{
	m_position = Vector2::ZERO;
	m_outerRadius = PLAYER_OUTER_RADIUS;
	m_innerRadius = PLAYER_INNER_RADIUS;
	m_maxHealth = DEFAULT_PLAYER_HEALTH;
	m_health = DEFAULT_PLAYER_HEALTH;
	m_entityType = ENTITY_TYPE_PLAYERTANK;
	m_faction = FACTION_PLAYER;

	SoundID startSound = g_theAudioSystem->CreateOrGetSound(PLAYER_TANK_ENGINE_SOUND_PATH);	
	m_engineSoundPlayback = g_theAudioSystem->PlaySound(startSound, true, 0.2f);
}


//-----------------------------------------------------------------------------------------------
// Destructor - for cleaning up
//
PlayerTank::~PlayerTank()
{
	// Stop playing the player tank engine when it is destroyed (back to main menu)
	g_theAudioSystem->StopSound(m_engineSoundPlayback);
}


//-----------------------------------------------------------------------------------------------
// Updates the PlayerTank's input-based fields from the last frame's user input
//
void PlayerTank::ProcessInputAndUpdate(float deltaTime)
{
	// Left stick input
	if (g_theInputSystem->GetController(0).GetCorrectedStickMagnitude(XBOX_STICK_LEFT) > 0)
	{
		// Check if we need to update the sound playing
		if (!m_isMoving)
		{	
			m_isMoving = true;
			g_theAudioSystem->SetSoundPlaybackSpeed(m_engineSoundPlayback, 1.2f);
		}

		// Turn the tank base
		float baseTargetOrientation = g_theInputSystem->GetController(0).GetStickOrientationDegrees(XBOX_STICK_LEFT);
		m_orientationDegrees = TurnToward(m_orientationDegrees, baseTargetOrientation, PLAYER_TANK_TURN_SPEED * deltaTime);

		// Translate the tank base
		Vector2 moveDirection = Vector2::MakeDirectionAtDegrees(m_orientationDegrees);
		m_position += (moveDirection * g_theInputSystem->GetController(0).GetCorrectedStickMagnitude(XBOX_STICK_LEFT) * m_moveSpeed * deltaTime);
	}
	else if (m_isMoving)
	{
		// We just stopped moving, so change to the idle sound
		m_isMoving = false;
		g_theAudioSystem->SetSoundPlaybackSpeed(m_engineSoundPlayback, (2.f / 3.f));
	}

	// Right Stick input
	if (g_theInputSystem->GetController(0).GetCorrectedStickMagnitude(XBOX_STICK_RIGHT) > 0)
	{
		// Rotate the turret
		float turretTargetOrientation = g_theInputSystem->GetController(0).GetStickOrientationDegrees(XBOX_STICK_RIGHT);
		m_turretOrientation = TurnToward(m_turretOrientation, turretTargetOrientation, PLAYER_TURRET_TURN_SPEED * deltaTime);
	}

	// Fire a bullet if either trigger is pulled more than half way
	float rightTriggerValue = g_theInputSystem->GetController(0).GetTriggerValue(XBOX_TRIGGER_RIGHT);
	float leftTriggerValue = g_theInputSystem->GetController(0).GetTriggerValue(XBOX_TRIGGER_LEFT);
	if (rightTriggerValue > 0.5f || leftTriggerValue > 0.5f)
	{
		// And only when the game is not paused
		if (deltaTime != 0.f)
		{
			FireBullet();
		}
	}

	// Toggle isUsingPhysics flag if F4 is pressed
	if (g_theInputSystem->WasKeyJustPressed(InputSystem::KEYBOARD_F4))
	{
		m_isUsingPhysics = !m_isUsingPhysics;
	}

	// Toggle god mode if F5 is pressed
	if (g_theInputSystem->WasKeyJustPressed(InputSystem::KEYBOARD_F5))
	{
		ToggleGodMode();
	}
}


//-----------------------------------------------------------------------------------------------
// Handles controller input and updates the tank's state each frame
//
void PlayerTank::Update(float deltaTime)
{
	Entity::Update(deltaTime);

	// Check for user input in updating this tank, if the player is not dead
	if (!m_isMarkedForDeath)
	{
		ProcessInputAndUpdate(deltaTime);
	}

	// Update the bullet fire rate cooldown
	m_bulletCooldown -= deltaTime;
	if (m_bulletCooldown < 0.f)
	{
		m_bulletCooldown = 0.f;
	}

	UpdateBreadCrumbs(deltaTime);

	// Check the speed boost timer
	m_speedTimer -= deltaTime;
	if (m_speedTimer <= 0.f)
	{
		m_speedTimer = 0.f;
		m_moveSpeed = PLAYER_TANK_DEFAULT_MOVE_SPEED;
	}
}


//-----------------------------------------------------------------------------------------------
// Draws the PlayerTank to screen - as a tank texture
//
void PlayerTank::Render() const
{
	// Set up coordinate system
	g_theRenderer->PushMatrix();

	// Translate a little for shake effect if the game isn't stopped and the player is moving
	Vector2 shakeOffset = Vector2::ZERO;
	if (!g_theGame->IsPaused() && !g_theGame->IsGameOver() && m_isMoving)
	{
		shakeOffset = Vector2::GetRandomVector(0.005f);
	}

	g_theRenderer->TranslateCoordinateSystem2D(m_position + shakeOffset);
	g_theRenderer->RotateCoordinateSystem2D(m_orientationDegrees);
	g_theRenderer->ScaleCoordinateSystem(m_outerRadius);

	// Gets the player textures
	Texture* baseTexture = g_theRenderer->CreateOrGetTexture(PLAYER_BASE_TEXTURE_PATH);
	Texture* turretTexture = g_theRenderer->CreateOrGetTexture(PLAYER_TURRET_TEXTURE_PATH);

	// Add a flashing effect for invincibility timer when spawned
	Rgba drawColor;
	if (IsInGodMode())
	{
		int flashNumber = static_cast<int>(m_age * PLAYER_INVINCIBILITY_FLASHES_PER_SECOND); 
		if (flashNumber % 2 == 0)
		{
			drawColor = Rgba::WHITE;
		}
		else
		{
			drawColor = Rgba::YELLOW;
		}
	}

	// Draw the texture, centered on the player position
	g_theRenderer->DrawTexturedAABB2(AABB2::UNIT_SQUARE_CENTERED, *baseTexture, Vector2(0.f, 1.f), Vector2(1.f, 0.f), drawColor);

	// Reset the coordinate system
	g_theRenderer->PopMatrix();

	// Draw the turret on top of the base, based on the bullet cooldown for a recoil effect
	Vector2 turretForwardDirection = Vector2::MakeDirectionAtDegrees(m_turretOrientation);
	Vector2 turretOffset = (turretForwardDirection * m_bulletCooldown * -1.0f);

	g_theRenderer->PushMatrix();
	g_theRenderer->TranslateCoordinateSystem2D(m_position + shakeOffset + turretOffset);
	g_theRenderer->RotateCoordinateSystem2D(m_turretOrientation);
	g_theRenderer->ScaleCoordinateSystem(m_outerRadius);

	g_theRenderer->DrawTexturedAABB2(AABB2::UNIT_SQUARE_CENTERED, *turretTexture, Vector2(0.f, 1.f), Vector2(1.f, 0.f), drawColor);


	// Reset the coordinate system
	g_theRenderer->PopMatrix();

	// Draw the health bar by calling Entity::Render()
	Entity::Render();

	// If the player is invincible, tint their health bar yellow
	if (m_inGodMode || m_age < PLAYER_INVINCIBILITY_DURATION)
	{
		Vector2 healthBarTopRight = HEALTH_BAR_BOTTOM_LEFT + HEALTH_BAR_DIMENSIONS;
		Rgba healthBarTint = Rgba::YELLOW;
		healthBarTint.a = 150;

		g_theRenderer->PushMatrix();
		g_theRenderer->TranslateCoordinateSystem2D(m_position);
		g_theRenderer->DrawAABB2(AABB2(HEALTH_BAR_BOTTOM_LEFT, healthBarTopRight), healthBarTint);
		g_theRenderer->PopMatrix();
	}
}


//-----------------------------------------------------------------------------------------------
// Toggles the god mode flag for the player, so they switch back and forth from being invincible
//
void PlayerTank::ToggleGodMode()
{
	m_inGodMode = !m_inGodMode;
}


//-----------------------------------------------------------------------------------------------
// Increases the player's speed for a short amount of time
//
void PlayerTank::ApplySpeedBoost(float speedMultiplier)
{
	// To prevent speed stacking
	if (m_moveSpeed == PLAYER_TANK_DEFAULT_MOVE_SPEED)
	{
		m_moveSpeed *= speedMultiplier;
	}
	
	// Add 5 seconds to the duration
	m_speedTimer += 5.0f;
}


//-----------------------------------------------------------------------------------------------
// Returns the deque of bread crumbs used for AI follow behavoir
//
std::vector<Vector2> PlayerTank::GetBreadCrumbs() const
{
	return m_breadcrumbs;
}


//-----------------------------------------------------------------------------------------------
// Returns true if the player is currently in god mode (invincible)
//
bool PlayerTank::IsInGodMode() const
{
	return (m_inGodMode || m_age < PLAYER_INVINCIBILITY_DURATION);
}


//-----------------------------------------------------------------------------------------------
// Performs the death animation when the player dies (player is not deleted)
//
void PlayerTank::PerformDeathAnimation() const
{
	// First stop our engine sound
	g_theAudioSystem->StopSound(m_engineSoundPlayback);

	// Spawn a bunch of sweet explosions when the player dies
	float randomDuration = GetRandomFloatInRange(0.2f, 2.0f);
	Vector2 randomOffset = Vector2::GetRandomVector(0.1f);

	for (int i = 0; i < 9; i++)
	{
		randomOffset = Vector2::GetRandomVector(0.1f);
		randomDuration = GetRandomFloatInRange(0.2f, 2.0f);
		m_map->SpawnExplosion(m_position + randomOffset, m_outerRadius * EXPLOSION_RADIUS_MULTIPLIER, randomDuration, false);
	}

	// Spawn one explosion with sound
	m_map->SpawnExplosion(m_position + randomOffset, m_outerRadius * EXPLOSION_RADIUS_MULTIPLIER, randomDuration, true);
}


//-----------------------------------------------------------------------------------------------
// Revives the player in the same position and orientation they died
//
void PlayerTank::Respawn()
{
	m_isMarkedForDeath = false;
	m_health = DEFAULT_PLAYER_HEALTH;
	m_ageAtDeath = -1;
	m_age = 0.f;

	// Start playing the engine sound again
	SoundID startSound = g_theAudioSystem->CreateOrGetSound(PLAYER_TANK_ENGINE_SOUND_PATH);
	m_engineSoundPlayback = g_theAudioSystem->PlaySound(startSound, true, 0.5f);

	// Play the respawn sound
	SoundID respawnSound = g_theAudioSystem->CreateOrGetSound(PLAYER_RESPAWN_SOUND_PATH);
	g_theAudioSystem->PlaySound(respawnSound);
}


//-----------------------------------------------------------------------------------------------
// Fires a bullet out of the front of the tank, if the cooldown is ready
//
void PlayerTank::FireBullet()
{
	if (m_bulletCooldown <= 0.f)
	{
		m_bulletCooldown = (1.f / PLAYER_BULLETS_PER_SECOND);

		// Use the inner radius for the magnitude to make it look like bullets are firing out the tip of the gun
		Vector2 firePosition = m_position + (Vector2::MakeDirectionAtDegrees(m_turretOrientation) * m_innerRadius * 1.1f);

		// Spawn the bullet
		m_map->SpawnEntity(firePosition, m_turretOrientation, m_faction, ENTITY_TYPE_BULLET);
		
		// Spawn a muzzle flare
		m_map->SpawnExplosion(firePosition, 0.1f, 0.05f, false);

		// Play the sound effect
		SoundID shotSound = g_theAudioSystem->CreateOrGetSound(SHOT_FIRED_SOUND_PATH);
		g_theAudioSystem->PlaySound(shotSound, false, 0.2f, 0.f, 0.7f);

	}
}


//-----------------------------------------------------------------------------------------------
// Decrements the timer and adds a breadcrumb if the timer is up
//
void PlayerTank::UpdateBreadCrumbs(float deltaTime)
{
	m_breadCrumbTimer -= deltaTime;
	if (m_breadCrumbTimer <= 0.f)
	{
		m_breadCrumbTimer = (1.f / NUM_BREADCRUMBS_PER_SECOND);
		m_breadcrumbs.push_back(m_position);
	}
}
