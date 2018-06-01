/************************************************************************/
/* File: Projectile.cpp
/* Author: Andrew Chase
/* Date: November 18th, 2017
/* Bugs: None
/* Description: Implementation of the Projectile class
/************************************************************************/
#include "Game/Projectile.hpp"
#include "Game/ProjectileDefinition.hpp"


//-----------------------------------------------------------------------------------------------
// Constructs a Projectile give the necessary Entity information and a ProjectileDefinition
//
Projectile::Projectile(const Vector2& position, float orientation, const ProjectileDefinition* definition, Map* entityMap, const std::string& name, const std::string& faction)
	: Entity(position, orientation, definition, entityMap, name)
	, m_faction(faction)
	, m_projectileDefinition(definition)
	, m_maxAge(definition->GetMaxAge())
{
	// Set the velocity of this projectile to be the direction it is fired
	Vector2 travelDirection = Vector2::MakeDirectionAtDegrees(orientation);
	float flySpeed = definition->GetFlySpeed();
	m_velocity = travelDirection * flySpeed;

	// Ensure projectiles only have one health
	m_health = 1;
}


//-----------------------------------------------------------------------------------------------
// Updates the projectile by calling Entity::Update()
//
void Projectile::Update(float deltaTime)
{
	Entity::Update(deltaTime);

	if (m_age >= m_maxAge && m_maxAge > 0.f)
	{
		m_isMarkedForDeletion = true;
	}
}


//-----------------------------------------------------------------------------------------------
// Draws the Projectile to screen using its Entity Animation data
//
void Projectile::Render() const
{
	// Rendering of the sprite animation is handled by the Entity class
	Entity::Render();

	// Draw additional Projectile-related things here
}


//-----------------------------------------------------------------------------------------------
// Returns the faction of this projectile (the faction of the actor who spawned this projectile)
//
std::string Projectile::GetFaction() const
{
	return m_faction;
}
