/************************************************************************/
/* Project: Incursion
/* File: NpcTank.cpp
/* Author: Andrew Chase
/* Date: October 7th, 2017
/* Bugs: None
/* Description: Implementation of the NpcTank class
/************************************************************************/
#include "Game/Game.hpp"
#include "Game/NpcTank.hpp"
#include "Game/PlayerTank.hpp"
#include "Game/Map.hpp"
#include "Game/Bullet.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"

// Static constants used for default NpcTank values
const float NpcTank::NPC_TANK_BULLETS_PER_SECOND			= 1.0f;
const float NpcTank::NPC_TANK_MIN_DOT_TO_DRIVE_FORWARD		= CosDegrees(45.f);
const float NpcTank::NPC_TANK_MIN_DOT_TO_SHOOT				= CosDegrees(5.f);
const float NpcTank::NPC_TANK_DEFAULT_VIEW_RANGE			= 20.f;
const float NpcTank::NPC_TANK_DEFAULT_MOVE_SPEED			= 1.0f;
const float NpcTank::NPC_TANK_DEFAULT_TURN_SPEED			= 90.f;
const int	NpcTank::NPC_TANK_DEFAULT_HEALTH				= 2;
const float NpcTank::NPC_MAX_PURSUE_DURATION				= 5.f;

//-----------------------------------------------------------------------------------------------
// Constructor - creates a different tank based on it's NpcTankType - currently NpcTankType is
// randomly chosen at construction
//
NpcTank::NpcTank(Vector2 spawnPosition, Faction entityFaction, NpcTankType tankType, Map* entityMap)
	: m_timeUntilNewWanderTarget(0.f)
	, m_wanderTargetOrientation(GetRandomFloatInRange(0.f, 360.f))
	, m_target(nullptr)
	, m_lastTargetPosition(Vector2(-1.f, -1.f))
	, m_hasTargetPosition(false)
	, m_pursueTimer(NPC_MAX_PURSUE_DURATION)
{

	m_faction = entityFaction;
	m_entityType = ENTITY_TYPE_NPCTANK;
	m_viewRange = NPC_TANK_DEFAULT_VIEW_RANGE;
	m_map = entityMap;

	m_position = spawnPosition;
	m_orientationDegrees = GetRandomFloatInRange(0.f, 360.f);

	// Assign a random tank type, and set its data members based on its type
	m_tankType = tankType;

	// Member variables that are based on the tank type
	switch (m_tankType)
	{
		// Lights have little health, but move quickly (suicide bombers)
	case NPC_TANK_TYPE_LIGHT:
		m_maxHealth = NPC_TANK_DEFAULT_HEALTH;
		m_health = NPC_TANK_DEFAULT_HEALTH;
		m_moveSpeed = 1.5f * NPC_TANK_DEFAULT_MOVE_SPEED;
		m_turnSpeed = 2.f * NPC_TANK_DEFAULT_TURN_SPEED;
		m_fireRateScalar = 0.5f;	
		m_outerRadius = 0.30f;
		m_innerRadius = 0.25f;
		break;
		// Mediums move slightly slower, but have more health
	case NPC_TANK_TYPE_MEDIUM:
		m_maxHealth = NPC_TANK_DEFAULT_HEALTH * 2;
		m_health = NPC_TANK_DEFAULT_HEALTH * 2;
		m_moveSpeed = NPC_TANK_DEFAULT_MOVE_SPEED;
		m_turnSpeed = NPC_TANK_DEFAULT_TURN_SPEED;
		m_fireRateScalar = 1.0f;
		m_outerRadius = 0.4f;
		m_innerRadius = 0.35f;
		break;
		// Heavy tanks are really slow, but have a decent amount of health
	case NPC_TANK_TYPE_HEAVY:
		m_maxHealth = NPC_TANK_DEFAULT_HEALTH * 5;
		m_health = NPC_TANK_DEFAULT_HEALTH * 5;
		m_moveSpeed = 0.5f * NPC_TANK_DEFAULT_MOVE_SPEED;
		m_turnSpeed = 0.5f * NPC_TANK_DEFAULT_TURN_SPEED;
		m_fireRateScalar = 1.5f;
		m_outerRadius = 0.6f;
		m_innerRadius = 0.4f;
		break;
		// Tank destroyers also have average health, but have firepower
	case NPC_TANK_TYPE_TD:
		m_maxHealth = NPC_TANK_DEFAULT_HEALTH * 2;
		m_health = NPC_TANK_DEFAULT_HEALTH * 2;
		m_moveSpeed = NPC_TANK_DEFAULT_MOVE_SPEED;
		m_turnSpeed = NPC_TANK_DEFAULT_TURN_SPEED;
		m_fireRateScalar = 3.0f;
		m_outerRadius = 0.4f;
		m_innerRadius = 0.35f;
		break;
	default:
		break;
	}

	m_bulletCooldown = (1.f / (NPC_TANK_BULLETS_PER_SECOND * m_fireRateScalar));

	// If the tank is an ally give it a lot more health
	if (m_faction == FACTION_PLAYER)
	{
		m_maxHealth *= 3;
		m_health = m_maxHealth;
	}

	m_map->AddEntityToMap(this);
}


//-----------------------------------------------------------------------------------------------
// Override destructor, just used to spawn an explosion on death
//
NpcTank::~NpcTank()
{
	if (!m_map->IsBeingDestroyed())
	{
		// Spawn a few explosions, with random offsets for effect (only play sound on one explosion)
		Vector2 randomOffset = Vector2::GetRandomVector(0.1f);
		m_map->SpawnExplosion(m_position + randomOffset, m_outerRadius * EXPLOSION_RADIUS_MULTIPLIER, 0.5f, true);

		randomOffset = Vector2::GetRandomVector(0.1f);
		m_map->SpawnExplosion(m_position + randomOffset, m_outerRadius * EXPLOSION_RADIUS_MULTIPLIER, 1.0f, false);

		randomOffset = Vector2::GetRandomVector(0.1f);
		m_map->SpawnExplosion(m_position + randomOffset, m_outerRadius * EXPLOSION_RADIUS_MULTIPLIER, 1.5f, false);

		// If the tank is a heavy tank, have it drop a pickup 33% of the time
		if (m_tankType == NPC_TANK_TYPE_HEAVY && CheckRandomChance(0.33f))
		{
			m_map->SpawnEntity(m_position, 0.f, FACTION_NEUTRAL, ENTITY_TYPE_PICKUP);
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Updates the tank based on its AI configuration, and whether it has Line of sight on the player
//
void NpcTank::Update(float deltaTime)
{
	// Call the base update to increment age
	Entity::Update(deltaTime);

	// If this tank is still alive
	if (!m_isMarkedForDeath)
	{
		m_target = m_map->FindBestTarget(this);

		// If the player is alive and we see them, target them
		if (m_target != nullptr)
		{
			m_hasTargetPosition = true;
			PerformAttackAI(deltaTime);
		}
		// Otherwise purse their last known position
		else if (m_hasTargetPosition)
		{
			PerformPursueAI(deltaTime);
		}
		// There is no last known position, so just wander or follow
		else
		{
			if (m_faction == FACTION_PLAYER)
			{
				bool couldFollowPlayer = PerformFollowAI(deltaTime);
				if (!couldFollowPlayer)
				{
					PerformWanderAI(deltaTime);
				}
			}
			else
			{
				PerformWanderAI(deltaTime);
			}
		}

		// Update timers each frame
		m_bulletCooldown -= deltaTime;
		m_timeUntilNewWanderTarget -= deltaTime;
	}	
}


//-----------------------------------------------------------------------------------------------
// Draws the tank to screen, choosing a different texture based on it's NpcTankType
//
void NpcTank::Render() const
{
	// Set up coordinate system
	g_theRenderer->PushMatrix();

	// Translate a little for shake effect, more so if the tank is moving
	Vector2 shakeOffset = Vector2::ZERO;
	if (!g_theGame->IsPaused() && !g_theGame->IsGameOver())
	{
		shakeOffset = Vector2::GetRandomVector(0.005f);
	}
	
	g_theRenderer->TranslateCoordinateSystem2D(m_position + shakeOffset);
	g_theRenderer->RotateCoordinateSystem2D(m_orientationDegrees);
	g_theRenderer->ScaleCoordinateSystem(m_outerRadius);

	// Gets the texture based on the tank type
	std::string tankTexture;
	switch (m_tankType)
	{
	case NPC_TANK_TYPE_LIGHT:
		if (m_faction != FACTION_PLAYER)
		{
			tankTexture = ENEMY_TANK_LIGHT_TEXTURE_PATH;
		}
		else
		{
			tankTexture = FRIENDLY_TANK_LIGHT_TEXTURE_PATH;
		}
		break;
	case NPC_TANK_TYPE_MEDIUM:
		if (m_faction != FACTION_PLAYER)
		{
			tankTexture = ENEMY_TANK_MEDIUM_TEXTURE_PATH;
		}
		else
		{
			tankTexture = FRIENDLY_TANK_MEDIUM_TEXTURE_PATH;
		}
		break;
	case NPC_TANK_TYPE_HEAVY:
		if (m_faction != FACTION_PLAYER)
		{
			tankTexture = ENEMY_TANK_HEAVY_TEXTURE_PATH;
		}
		else
		{
			tankTexture = FRIENDLY_TANK_HEAVY_TEXTURE_PATH;
		}
		break;
	case NPC_TANK_TYPE_TD:
		if (m_faction != FACTION_PLAYER)
		{
			tankTexture = ENEMY_TANK_TD_TEXTURE_PATH;
		}
		else
		{
			tankTexture = FRIENDLY_TANK_TD_TEXTURE_PATH;
		}
		break;
	default:
		break;
	}
	Texture* enemyTankTexture = g_theRenderer->CreateOrGetTexture(tankTexture);

	// Draw the texture, centered on the tank's position
	g_theRenderer->DrawTexturedAABB2(AABB2::UNIT_SQUARE_CENTERED, *enemyTankTexture, Vector2(0.f, 1.f), Vector2(1.f, 0.f), Rgba::WHITE);

	// Reset the coordinate system
	g_theRenderer->PopMatrix();

	// Draw the health bar by calling Entity::Render()
	Entity::Render();
}


//-----------------------------------------------------------------------------------------------
// Draws the tank to screen, choosing a different texture based on it's NpcTankType
//
void NpcTank::PerformAttackAI(float deltaTime)
{
	// Update the last known target position
	m_lastTargetPosition = m_target->GetPosition();

	Vector2 directionToTarget = (m_target->GetPosition() - m_position).GetNormalized();
	float angleToTarget = directionToTarget.GetOrientationDegrees();

	// Turn towards target
	m_orientationDegrees = TurnToward(m_orientationDegrees, angleToTarget, m_turnSpeed * deltaTime);

	// Get the dot product for further AI behavior
	Vector2 forwardDirection = Vector2::MakeDirectionAtDegrees(m_orientationDegrees);
	float dotToTarget = DotProduct(directionToTarget, forwardDirection);

	// If player is within 90 degree frontal cone, drive forward
	if (dotToTarget > NPC_TANK_MIN_DOT_TO_DRIVE_FORWARD)
	{
		m_position += forwardDirection * deltaTime * m_moveSpeed;
	}

	// If player is within 10 degree frontal cone, shoot a bullet if off cooldown
	if (dotToTarget > NPC_TANK_MIN_DOT_TO_SHOOT)
	{
		ShootWithCooldown();
	}
}


//-----------------------------------------------------------------------------------------------
// Moves the tank towards its last known (hostile) target position
//
void NpcTank::PerformPursueAI(float deltaTime)
{
	Vector2 targetDirection = (m_lastTargetPosition - m_position).GetNormalized();
	float targetOrientation = targetDirection.GetOrientationDegrees();

	m_orientationDegrees = TurnToward(m_orientationDegrees, targetOrientation, m_turnSpeed * deltaTime);

	// Get the dot product for further AI behavior
	Vector2 forwardDirection = Vector2::MakeDirectionAtDegrees(m_orientationDegrees);
	float dotToTarget = DotProduct(targetDirection, forwardDirection);

	// If player is within 90 degree frontal cone, drive forward
	if (dotToTarget > NPC_TANK_MIN_DOT_TO_DRIVE_FORWARD)
	{
		m_position += forwardDirection * deltaTime * m_moveSpeed;
	}

	float distanceToTarget = (m_lastTargetPosition - m_position).GetLength();

	if (distanceToTarget < m_outerRadius || m_pursueTimer <= 0.f)
	{
		m_hasTargetPosition = false;
		m_pursueTimer = NPC_MAX_PURSUE_DURATION;
	}
	else
	{
		m_pursueTimer -= deltaTime;
	}
}


//-----------------------------------------------------------------------------------------------
// Moves the tank forward in a random orientation direction, switching the direction every two seconds
//
void NpcTank::PerformWanderAI(float deltaTime)
{
	// Select a new target if the wander timer is up
	if (m_timeUntilNewWanderTarget < 0.f)
	{
		// Change direction
		m_wanderTargetOrientation = GetRandomFloatInRange(0.f, 360.f);
		m_timeUntilNewWanderTarget = 2.f;
	}

	// Turn the tank
	m_orientationDegrees = TurnToward(m_orientationDegrees, m_wanderTargetOrientation, m_turnSpeed * deltaTime);

	// Move the tank forward
	m_position += Vector2::MakeDirectionAtDegrees(m_orientationDegrees) * deltaTime * m_moveSpeed;
}


//-----------------------------------------------------------------------------------------------
// Follow the player's trail of breadcrumbs, returns true if a breadcrumb is seen
//
bool NpcTank::PerformFollowAI(float deltaTime)
{
	std::vector<Vector2> playerBreadCrumbs = g_thePlayer->GetBreadCrumbs();

	for (int breadCrumbIndex = static_cast<int>(playerBreadCrumbs.size()) - 1; breadCrumbIndex >= 0; breadCrumbIndex--)
	{
		Vector2 breadCrumbPosition = playerBreadCrumbs[breadCrumbIndex];
		float distanceBetween = (breadCrumbPosition - m_position).GetLength();

		if (m_map->HasLineOfSight(m_position, breadCrumbPosition, distanceBetween + 1.f))
		{
			// Move towards that position, and return true
			Vector2 targetDirection = (breadCrumbPosition - m_position).GetNormalized();
			float targetOrientation = targetDirection.GetOrientationDegrees();

			m_orientationDegrees = TurnToward(m_orientationDegrees, targetOrientation, m_turnSpeed * deltaTime);

			// Get the dot product for further AI behavior
			Vector2 forwardDirection = Vector2::MakeDirectionAtDegrees(m_orientationDegrees);
			float dotToTarget = DotProduct(targetDirection, forwardDirection);

			// If player is within 90 degree frontal cone, drive forward
			if (dotToTarget > NPC_TANK_MIN_DOT_TO_DRIVE_FORWARD)
			{
				m_position += forwardDirection * deltaTime * m_moveSpeed;
			}
			return true;
		}
	}

	return false;

}


//-----------------------------------------------------------------------------------------------
// Shoots a bullet out the front of the tank, if the tank is off cooldown
//
void NpcTank::ShootWithCooldown()
{
	if (m_bulletCooldown <= 0.f)
	{
		// Reset the timer
		m_bulletCooldown = (1.f / NPC_TANK_BULLETS_PER_SECOND);

		// Use the inner radius for the magnitude to make it look like bullets are firing out the tip of the gun
		Vector2 firePosition = m_position + (Vector2::MakeDirectionAtDegrees(m_orientationDegrees) * m_innerRadius);
		m_map->SpawnEntity(firePosition, m_orientationDegrees, m_faction, ENTITY_TYPE_BULLET);

		// Play the sound effect
		SoundID shotSound = g_theAudioSystem->CreateOrGetSound(SHOT_FIRED_SOUND_PATH);
		g_theAudioSystem->PlaySound(shotSound, false, 0.2f, 0.f, 1.5f);
	}
}
