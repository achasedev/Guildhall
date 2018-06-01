/************************************************************************/
/* Project: Incursion
/* File: NpcTurret.cpp
/* Author: Andrew Chase
/* Date: October 10th, 2017
/* Bugs: None
/* Description: Implementation of the NpcTurret class
/************************************************************************/
#include "Game/NpcTurret.hpp"
#include "Game/PlayerTank.hpp"
#include "Game/Map.hpp"
#include "Game/Bullet.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"

// Constants for controlling turret behavior
const int	NpcTurret::NPC_TURRET_DEFAULT_HEALTH		= 8;
const float NpcTurret::NPC_TURRET_DEFAULT_VIEW_RANGE	= 20.f;
const float NpcTurret::NPC_TURRET_DEFAULT_TURN_SPEED	= 30.f;
const float NpcTurret::NPC_TURRET_BULLETS_PER_SECOND	= 1.5f;
const float NpcTurret::NPC_TURRET_MIN_DOT_TO_SHOOT		= CosDegrees(5.f);
const float NpcTurret::NPC_TURRET_IDLE_TURN_LIMIT		= 45.f;

//-----------------------------------------------------------------------------------------------
// Main constructor - creates a turret given the position and faction, and adds it to the map
//
NpcTurret::NpcTurret(Vector2 spawnPosition, Faction entityFaction, Map* entityMap)
	: m_target(nullptr)
	, m_idleTurnDirection(1)
	, m_lastTargetOrientation(0)
{
	m_outerRadius = 0.5f;
	m_innerRadius = 0.4f;

	m_faction = entityFaction;
	m_entityType = ENTITY_TYPE_TURRET;
	m_map = entityMap;

	m_position = spawnPosition;
	m_orientationDegrees = GetRandomFloatInRange(0.f, 359.f);

	m_bulletCooldown = (1.f / NPC_TURRET_BULLETS_PER_SECOND);
	m_maxHealth = NPC_TURRET_DEFAULT_HEALTH;
	m_health = NPC_TURRET_DEFAULT_HEALTH;
	m_map->AddEntityToMap(this);
}


//-----------------------------------------------------------------------------------------------
// Overridden destructor, used to spawn an explosion on death
//
NpcTurret::~NpcTurret()
{
	if (!m_map->IsBeingDestroyed())
	{
		// Spawn a few explosions, with random offsets for effect
		Vector2 randomOffset = Vector2::GetRandomVector(0.1f);
		m_map->SpawnExplosion(m_position + randomOffset, m_outerRadius * EXPLOSION_RADIUS_MULTIPLIER, 0.5f, true);

		randomOffset = Vector2::GetRandomVector(0.1f);
		m_map->SpawnExplosion(m_position + randomOffset, m_outerRadius * EXPLOSION_RADIUS_MULTIPLIER, 1.0f, false);

		randomOffset = Vector2::GetRandomVector(0.1f);
		m_map->SpawnExplosion(m_position + randomOffset, m_outerRadius * EXPLOSION_RADIUS_MULTIPLIER, 2.0f, false);

		// Have turrets drop a pickup 20% of the time
		if (CheckRandomChance(0.2f))
		{
			m_map->SpawnEntity(m_position, 0.f, FACTION_NEUTRAL, ENTITY_TYPE_PICKUP);
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Updates the turret by performing basic AI if the player is in range and has line of sight
//
void NpcTurret::Update(float deltaTime)
{
	// Call the base update to increment age
	Entity::Update(deltaTime);

	// If we're still alive
	if (!m_isMarkedForDeath)
	{
		m_target = m_map->FindBestTarget(this);

		// If the player is alive and we see them, target them
		if (m_target != nullptr)
		{
			PerformTargetAI(deltaTime);
		}
		else
		{
			PerformIdleBehavior(deltaTime);
		}

		// Update timers each frame
		m_bulletCooldown -= deltaTime;
	}
}


//-----------------------------------------------------------------------------------------------
// Draws the turret to screen as two separate textures - a gun cannon on a circular base
//
void NpcTurret::Render() const
{
	// Set up coordinate system
	g_theRenderer->PushMatrix();
	g_theRenderer->TranslateCoordinateSystem2D(m_position.x, m_position.y);
	g_theRenderer->ScaleCoordinateSystem(m_outerRadius);

	// Gets the texture based on the tank type
	Texture* turretBaseTexture = g_theRenderer->CreateOrGetTexture(TURRET_BASE_TEXTURE_PATH);
	Texture* turretTopTexture = g_theRenderer->CreateOrGetTexture(TURRET_TOP_TEXTURE_PATH);

	// Draw the base without rotation
	g_theRenderer->DrawTexturedAABB2(AABB2::UNIT_SQUARE_CENTERED, *turretBaseTexture, Vector2(0.f, 1.f), Vector2(1.f, 0.f), Rgba::WHITE);

	// Rotate the coordinate system to draw the gun
	g_theRenderer->RotateCoordinateSystem2D(m_orientationDegrees);

	// Draw the gun
	g_theRenderer->DrawTexturedAABB2(AABB2::UNIT_SQUARE_CENTERED, *turretTopTexture, Vector2(0.f, 1.f), Vector2(1.f, 0.f), Rgba::WHITE);

	// Reset the coordinate system
	g_theRenderer->PopMatrix();

	// Draw the turret's laser sight, in global coordinates
	Vector2 raycastDirection = Vector2::MakeDirectionAtDegrees(m_orientationDegrees);
	
	Vector2 startPosition = (m_position + (raycastDirection * m_outerRadius * 0.95f)); // Scale by 0.95 so the laser starts at the tip of the gun nicely
	RaycastResult laserRaycastResult = m_map->Raycast(startPosition, raycastDirection, NPC_TURRET_DEFAULT_VIEW_RANGE);


	Vector2 endPosition = laserRaycastResult.m_impactPosition;
	float alphaScale = (1.0f - laserRaycastResult.m_impactFraction);
	Rgba startColor = Rgba::RED;
	Rgba endColor = Rgba::RED;
	endColor.ScaleAlpha(alphaScale);

	g_theRenderer->DrawLineColor(startPosition, startColor, endPosition, endColor);


	// Draw the health bar by calling Entity::Render()
	Entity::Render();
}


//-----------------------------------------------------------------------------------------------
// Rotates the gun towards the player, and fires if the player is within the turret's frontal targeting
// cone
//
void NpcTurret::PerformTargetAI(float deltaTime)
{
	// Update the last known target orientation
	Vector2 directionToTarget = (m_target->GetPosition() - m_position).GetNormalized();
	m_lastTargetOrientation = directionToTarget.GetOrientationDegrees();

	// Turn towards target
	m_orientationDegrees = TurnToward(m_orientationDegrees, m_lastTargetOrientation, NPC_TURRET_DEFAULT_TURN_SPEED * deltaTime);

	// Get the dot product for further AI behavior
	Vector2 forwardDirection = Vector2::MakeDirectionAtDegrees(m_orientationDegrees);
	float dotToTarget = DotProduct(directionToTarget, forwardDirection);

	// If player is within 10 degree frontal cone, shoot a bullet if off cooldown
	if (dotToTarget > NPC_TURRET_MIN_DOT_TO_SHOOT)
	{
		ShootWithCooldown();
	}
}


//-----------------------------------------------------------------------------------------------
// Turns the turret back and forth on the last orientation of a hostile target
//
void NpcTurret::PerformIdleBehavior(float deltaTime)
{
	// Calculate our goal orientation (offset from the last target orientation)
	float offset = static_cast<float>(m_idleTurnDirection) * NPC_TURRET_IDLE_TURN_LIMIT;
	float goalOrientation = m_lastTargetOrientation + offset;

	// Turn the turret
	m_orientationDegrees = TurnToward(m_orientationDegrees, goalOrientation, NPC_TURRET_DEFAULT_TURN_SPEED * deltaTime);

	// If we reached our limit, begin turning the other direction
	if (m_orientationDegrees == goalOrientation)
	{
		m_idleTurnDirection *= -1;
	}
}


//-----------------------------------------------------------------------------------------------
// Finds the nearest hostile target that is in line of sight to this Entity.
// Returns nullptr if no such target exists
//
Entity* NpcTurret::FindTarget()
{
	return m_map->FindBestTarget(this);
}

//-----------------------------------------------------------------------------------------------
// Shoots a bullet out the front of the turret, if the turret is off cooldown
//
void NpcTurret::ShootWithCooldown()
{
	if (m_bulletCooldown <= 0.f)
	{
		m_bulletCooldown = (1.f / NPC_TURRET_BULLETS_PER_SECOND);

		Vector2 firePosition = m_position + (Vector2::MakeDirectionAtDegrees(m_orientationDegrees) * m_outerRadius);
		m_map->SpawnEntity(firePosition, m_orientationDegrees, m_faction, ENTITY_TYPE_BULLET);

		// Play the sound effect
		SoundID shotSound = g_theAudioSystem->CreateOrGetSound(SHOT_FIRED_SOUND_PATH);
		g_theAudioSystem->PlaySound(shotSound, false, 0.2f, 0.f, 1.5f);
	}
}
