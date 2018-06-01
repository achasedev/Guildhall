/************************************************************************/
/* Project: Incursion
/* File: Entity.cpp
/* Author: Andrew Chase
/* Date: September 23rd, 2017
/* Bugs: None
/* Description: Implementation of the Entity class
/************************************************************************/
#include "Game/Entity.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Map.hpp"
#include "Game/PlayerTank.hpp"
#include "Game/Camera.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Audio/AudioSystem.hpp"

// Size of an explosion relative to the outer radius of the entity
const float Entity::EXPLOSION_RADIUS_MULTIPLIER		= 1.5f;
const Vector2 Entity::HEALTH_BAR_BOTTOM_LEFT		= Vector2(-0.25f, 0.4f);
const Vector2 Entity::HEALTH_BAR_DIMENSIONS			= Vector2(0.5f,  0.1f);

//-----------------------------------------------------------------------------------------------
// This constructor should NEVER be used explicitly - Entity is an abstract class
//
Entity::Entity()
	: m_position(Vector2::ZERO)
	, m_orientationDegrees(0.f)
	, m_outerRadius(0.f)
	, m_innerRadius(0.f)
	, m_isMarkedForDeath(false)
	, m_maxHealth(0)
	, m_health(0)
	, m_isUsingPhysics(true)
	, m_age(0.f)
	, m_ageAtDeath(-1.f)
	, m_faction(FACTION_ERROR)
	, m_entityType(ENTITY_TYPE_ERROR)
	, m_map(nullptr)
{
}


//-----------------------------------------------------------------------------------------------
// Overridden Destructor, used to delete derived instances of Entity
//
Entity::~Entity()
{
}


//-----------------------------------------------------------------------------------------------
// Base Update - only updates the Entity's age
//
void Entity::Update(float deltaTime)
{
	m_age += deltaTime;
}


//-----------------------------------------------------------------------------------------------
// Draws a health bar above the entity
//
void Entity::Render() const
{
	// Set up coordinate system
	g_theRenderer->PushMatrix();
	g_theRenderer->TranslateCoordinateSystem2D(m_position);

	// Drawing the outline
	Vector2 outlineTopRight = HEALTH_BAR_BOTTOM_LEFT + HEALTH_BAR_DIMENSIONS;
	g_theRenderer->DrawAABB2(AABB2(HEALTH_BAR_BOTTOM_LEFT, outlineTopRight), Rgba::BLACK);

	// Drawing the red backdrop
	Vector2 redBarBottomLeft = HEALTH_BAR_BOTTOM_LEFT + Vector2(0.025f, 0.025f);
	Vector2 redBarTopRight = outlineTopRight - Vector2(0.025f, 0.025f);
	g_theRenderer->DrawAABB2(AABB2(redBarBottomLeft, redBarTopRight), Rgba::RED);
	
	// Draw the green portion that represents remaining health
	float fractionHealthRemaining = static_cast<float>(m_health) / static_cast<float>(m_maxHealth);
	float interiorBarLength = redBarTopRight.x - redBarBottomLeft.x;
	float greenBarLength = fractionHealthRemaining * interiorBarLength;

	Vector2 greenBarTopRight = Vector2(redBarBottomLeft.x + greenBarLength, redBarTopRight.y);

	g_theRenderer->DrawAABB2(AABB2(redBarBottomLeft, greenBarTopRight), Rgba::LIGHT_GREEN);
	g_theRenderer->PopMatrix();
}


//-----------------------------------------------------------------------------------------------
// Sets the 2D world position of the object
//
void Entity::SetPosition(const Vector2& newPosition)
{
	if (m_entityType == ENTITY_TYPE_PLAYERTANK)
	{
		m_position = newPosition;

	}
	else
	{
		m_position = newPosition;
	}
}


//-----------------------------------------------------------------------------------------------
// Sets the rotation angle of the object of the object
//
void Entity::SetOrientationDegrees(float newOrientationDegress)
{
	m_orientationDegrees = newOrientationDegress;
}


//-----------------------------------------------------------------------------------------------
// Sets the physics radius of the object
//
void Entity::SetInnerRadius(float newInnerRadius)
{
	m_innerRadius = newInnerRadius;
}


//-----------------------------------------------------------------------------------------------
// Sets the cosmetic radius of the object
//
void Entity::SetOuterRadius(float newOuterRadius)
{
	m_outerRadius = newOuterRadius;
}


//-----------------------------------------------------------------------------------------------
// Sets the Entity's marked for death flag to the bool specified
//
void Entity::SetMarkedForDeath(bool markedForDeath)
{
	m_isMarkedForDeath = markedForDeath;
	if (markedForDeath == true)
	{
		m_ageAtDeath = m_age;
	}
	else
	{
		m_ageAtDeath = -1.f;
	}
}


//-----------------------------------------------------------------------------------------------
// Sets the Entity's age to the one specified
//
void Entity::SetAge(float newAge)
{
	m_age = newAge;
}


//-----------------------------------------------------------------------------------------------
// Sets the Entity's age at death to the one specified
//
void Entity::SetAgeAtDeath(float ageAtDeath)
{
	m_ageAtDeath = ageAtDeath;
}


//-----------------------------------------------------------------------------------------------
// Sets the faction of this entity to the one specified
//
void Entity::SetFaction(Faction newFaction)
{
	m_faction = newFaction;
}


//-----------------------------------------------------------------------------------------------
// Sets the type of this entity to the one specified
//
void Entity::SetEntityType(EntityType newType)
{
	m_entityType = newType;
}


//-----------------------------------------------------------------------------------------------
// Sets the map member of this entity to the pointer provided
//
void Entity::SetMap(Map* entityMap)
{
	m_map = entityMap;
}


//-----------------------------------------------------------------------------------------------
// Sets the physics flag for this entity to isUsingPhysics
//
void Entity::SetIsUsingPhysics(bool isUsingPhysics)
{
	m_isUsingPhysics = isUsingPhysics;
}


//-----------------------------------------------------------------------------------------------
// Adds health to the entity's current health, clamping at m_maxHealth
//
void Entity::AddHealth(int healthToAdd)
{
	m_health += healthToAdd;
	if (m_health > m_maxHealth)
	{
		m_health = m_maxHealth;
	}
}


//-----------------------------------------------------------------------------------------------
// Decrements this Entity's health by damageAmount, clamping to zero
//
void Entity::TakeDamage(int damageAmount)
{
	m_health -= damageAmount;
	if (m_health <= 0)
	{
		m_health = 0;
		m_isMarkedForDeath = true;
		m_ageAtDeath = m_age;
	}

	if (m_entityType == ENTITY_TYPE_PLAYERTANK)
	{
		// Play the sound effect for the player taking damage and perform screen shake
		SoundID damageSound = g_theAudioSystem->CreateOrGetSound(PLAYER_DAMAGE_TAKEN_SOUND_PATH);
		g_theAudioSystem->PlaySound(damageSound, false, GetRandomFloatInRange(0.8f, 1.0f), 0.f, GetRandomFloatInRange(0.5f, 1.5f));

		g_theCamera->StartScreenShake(0.25f, 0.05f);
	}
	else if (m_entityType != ENTITY_TYPE_BULLET)
	{
		// Only play the sound if the enemy is near the player (on screen)
		float distanceToPlayer = (g_thePlayer->GetPosition() - m_position).GetLength();

		if (distanceToPlayer < 7.f)
		{
			SoundID damageSound = g_theAudioSystem->CreateOrGetSound(ENEMY_DAMAGE_TAKEN_SOUND_PATH);
			g_theAudioSystem->PlaySound(damageSound, false, GetRandomFloatInRange(0.1f, 0.3f), 0.f, GetRandomFloatInRange(0.5f, 1.5f));
		}

	}
}

//-----------------------------------------------------------------------------------------------
// Returns the 2D position of the object
//
Vector2 Entity::GetPosition() const
{
	return m_position;
}


//-----------------------------------------------------------------------------------------------
// Returns the rotation orientation of the object, in degrees
//
float Entity::GetOrientationDegrees() const
{
	return m_orientationDegrees;
}


//-----------------------------------------------------------------------------------------------
// Returns the physics radius of the object
//
float Entity::GetInnerRadius() const
{
	return m_innerRadius;
}


//-----------------------------------------------------------------------------------------------
// Returns the cosmetic radius of the object
//
float Entity::GetOuterRadius() const
{
	return m_outerRadius;
}


//-----------------------------------------------------------------------------------------------
// Checks if the object is moving offscreen and if so sets its position to wrap around
//
bool Entity::IsMarkedForDeath() const
{
	return m_isMarkedForDeath;
}


//-----------------------------------------------------------------------------------------------
// Returns the age of this entity
//
float Entity::GetAge() const
{
	return m_age;
}


//-----------------------------------------------------------------------------------------------
// Returns the age this entity was at death, or -1.f if the entity hasn't died yet
//
float Entity::GetAgeAtDeath() const
{
	return m_ageAtDeath;
}


//-----------------------------------------------------------------------------------------------
// Returns the faction of this entity
//
Faction Entity::GetFaction() const
{
	return m_faction;
}


//-----------------------------------------------------------------------------------------------
// Returns the type of this entity
//
EntityType Entity::GetEntityType() const
{
	return m_entityType;
}

//-----------------------------------------------------------------------------------------------
// Returns true if this entity collides with tiles and other entities
//
bool Entity::IsUsingPhysics() const
{
	return m_isUsingPhysics;
}


//-----------------------------------------------------------------------------------------------
// Draws the Entity debug information
//
void Entity::DrawDebugInfo() const
{
	// Draw inner radius in cyan
	g_theRenderer->SetDrawColor(Rgba::CYAN);
	g_theRenderer->DrawDottedCircle(m_position, m_innerRadius);

	// Draw outer radius in magenta
	g_theRenderer->SetDrawColor(Rgba::MAGENTA);
	g_theRenderer->DrawDottedCircle(m_position, m_outerRadius);

	// Important - set color back to white
	g_theRenderer->SetDrawColor(Rgba::WHITE);
}
