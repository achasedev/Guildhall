/************************************************************************/
/* Project: Incursion
/* File: Bullet.cpp
/* Author: Andrew Chase
/* Date: October 7th, 2017
/* Bugs: None
/* Description: Implementation of the Bullet class
/************************************************************************/
#include "Game/Bullet.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Map.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/MathUtils.hpp"


const float Bullet::DEFAULT_BULLET_SPEED = 3.f;
const float Bullet::BULLET_OUTER_RADIUS = 0.08f;
const float Bullet::BULLET_INNER_RADIUS = 0.03f;

//-----------------------------------------------------------------------------------------------
// Constructor - Spawns a new bullet with the given params and adds it to the current map's entity list
//
Bullet::Bullet(Vector2 position, float orientation, Faction entityFaction, Map* currentMap)
{
	m_position = position;
	m_orientationDegrees = orientation;
	m_outerRadius = BULLET_OUTER_RADIUS;
	m_innerRadius = BULLET_INNER_RADIUS;

	// Player bullets get a health of 3, and move faster
	if (entityFaction == FACTION_PLAYER)
	{
		m_maxHealth = 3;
		m_health = 3;
		m_speed = (2.f * DEFAULT_BULLET_SPEED);
	}
	else
	{
		m_maxHealth = 1;
		m_health = 1;
		m_speed = DEFAULT_BULLET_SPEED;
	}

	m_velocity = (Vector2::MakeDirectionAtDegrees(m_orientationDegrees) * m_speed);

	m_faction = entityFaction;
	m_entityType = ENTITY_TYPE_BULLET;
	m_map = currentMap;
	m_map->AddEntityToMap(this);
}


Bullet::~Bullet()
{
	// No sound effect for bullet explosions
	m_map->SpawnExplosion(m_position, m_outerRadius * EXPLOSION_RADIUS_MULTIPLIER, 0.5f, false);
}

//-----------------------------------------------------------------------------------------------
// Updates the bullet by moving it in the direction it's facing, at the default bullet speed
// Also uses preventative physics, and only moves the bullet if the next position is valid, and
// bounces the bullet off the surface otherwise
//
void Bullet::Update(float deltaTime)
{
	Entity::Update(deltaTime);

	if (!m_isMarkedForDeath)
	{
		Vector2 nextPosition = m_position + (m_velocity * deltaTime);

		if (!m_map->GetTileFromPosition(nextPosition).CanSeeThrough())
		{
			// Don't move the bullet, but instead change its moving direction
			// Raycast to find the impact position
			float rayDistance = (nextPosition - m_position).GetLength();
			RaycastResult raycastResult = m_map->Raycast(m_position, m_velocity, rayDistance);

			float speedInNormalDirection = DotProduct(m_velocity, raycastResult.m_impactNormal);
			Vector2 normalVelocity = (raycastResult.m_impactNormal * speedInNormalDirection);

			// Update our velocity and orientation to reflect the change, and decrement health
			m_velocity -= (2.f * normalVelocity);
			m_orientationDegrees = m_velocity.GetOrientationDegrees();
			TakeDamage(1);
		}
		else
		{
			m_position += (m_velocity * deltaTime);
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Draws the bullet to screen, tinting the color based on its faction
// Blue - player, Red - Enemy, Neutral - Green
//
void Bullet::Render() const
{
	// Set up coordinate system
	g_theRenderer->PushMatrix();
	g_theRenderer->TranslateCoordinateSystem2D(m_position.x, m_position.y);
	g_theRenderer->RotateCoordinateSystem2D(m_orientationDegrees);
	g_theRenderer->ScaleCoordinateSystem(m_outerRadius);

	// Gets the player textures
	Texture* bulletTexture = g_theRenderer->CreateOrGetTexture(BULLET_TEXTURE_PATH);

	// Tint the bullet color based on it's faction
	Rgba bulletColor;

	if (m_faction == FACTION_PLAYER)
	{
		bulletColor = Rgba::LIGHT_BLUE;
	}
	else if (m_faction == FACTION_ENEMY)
	{
		bulletColor = Rgba::RED;
	}
	else if (m_faction == FACTION_NEUTRAL)
	{
		bulletColor = Rgba::LIGHT_GREEN;
	}
	else
	{
		bulletColor = Rgba::BLACK;
	}

	if (m_faction == FACTION_PLAYER)
	{
		g_theRenderer->PushMatrix();
		g_theRenderer->ScaleCoordinateSystem(1.2f);
		g_theRenderer->DrawTexturedAABB2(AABB2::UNIT_SQUARE_CENTERED, *bulletTexture, Vector2(0.f, 1.f), Vector2(1.f, 0.f), Rgba::YELLOW);
		g_theRenderer->PopMatrix();
	}
	// Draw the texture, centered on the bullet position
	g_theRenderer->DrawTexturedAABB2(AABB2::UNIT_SQUARE_CENTERED, *bulletTexture, Vector2(0.f, 1.f), Vector2(1.f, 0.f), bulletColor);

	// Reset the coordinate system
	g_theRenderer->PopMatrix();
}


//-----------------------------------------------------------------------------------------------
// Calculates and returns the bullet's next move position, based on it's orientation and speed
//
Vector2 Bullet::GetNextPosition() const
{
	Vector2 direction = Vector2::MakeDirectionAtDegrees(m_orientationDegrees);
	return m_position + (direction * DEFAULT_BULLET_SPEED);
}
