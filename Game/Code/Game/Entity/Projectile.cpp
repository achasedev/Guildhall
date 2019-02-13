/************************************************************************/
/* File: Projectile.cpp
/* Author: Andrew Chase
/* Date: September 28th 2018
/* Description: Implementation of the projectile class
/************************************************************************/
#include "Game/Framework/Game.hpp"
#include "Game/Framework/World.hpp"
#include "Game/Entity/Projectile.hpp"
#include "Game/Animation/VoxelSprite.hpp"
#include "Game/Entity/EntityDefinition.hpp"
#include "Engine/Core/Time/Stopwatch.hpp"


//-----------------------------------------------------------------------------------------------
// Constructor from a definition
//
Projectile::Projectile(const EntityDefinition* definition, eEntityTeam team)
	: Entity(definition)
{
	m_entityTeam = team;

	if (team == ENTITY_TEAM_PLAYER)
	{
		m_collisionLayerOverride = COLLISION_LAYER_PLAYER_BULLET;
	}
	else
	{
		m_collisionLayerOverride = COLLISION_LAYER_ENEMY_BULLET;
	}

	m_useCollisionLayerOverride = true;

	m_stopwatch = new Stopwatch(Game::GetGameClock());
	m_shouldCheckForEdgeCollisions = false;
}


//-----------------------------------------------------------------------------------------------
// Update
//
void Projectile::Update()
{
	Entity::Update();

	if (!IsMarkedForDelete() && m_stopwatch->HasIntervalElapsed())
	{
		m_isMarkedForDelete = true;
	}
}


//-----------------------------------------------------------------------------------------------
// Returns the texture to use based on the orientation of the projectile
//
const VoxelSprite* Projectile::GetVoxelSprite() const
{
	return m_definition->m_defaultSprite;
}


//-----------------------------------------------------------------------------------------------
// On Collision event
//
void Projectile::OnEntityCollision(Entity* other)
{
	if (other->GetTeam() != m_entityTeam && !other->IsMarkedForDelete())
	{
		Entity::OnEntityCollision(other);
		Vector3 direction = (other->GetCenterPosition() - GetCenterPosition()).GetNormalized();
		other->TakeDamage(m_definition->m_projectileDamage, m_definition->m_collisionDef.m_collisionKnockback * direction);

		if (m_definition->m_projectileHitRadius > 0.f)
		{
			Game::GetWorld()->ApplyExplosion(GetCoordinatePosition(), m_entityTeam, m_definition->m_projectileDamage, m_definition->m_projectileHitRadius, m_definition->m_collisionDef.m_collisionKnockback, other);
		}
		
		// Projectiles are only good for one collision
		m_isMarkedForDelete = true;
	}
}


//-----------------------------------------------------------------------------------------------
// Destroys the map the projectile hit and kills the projectile
//
void Projectile::OnGroundCollision()
{
	Game::GetWorld()->ApplyExplosion(GetCoordinatePosition(), m_entityTeam, m_definition->m_projectileDamage, m_definition->m_projectileHitRadius, m_definition->m_collisionDef.m_collisionKnockback);
	m_isMarkedForDelete = true;
}


//-----------------------------------------------------------------------------------------------
// On damage taken event
//
void Projectile::OnDamageTaken(int damageAmount)
{
	Entity::OnDamageTaken(damageAmount);
}


//-----------------------------------------------------------------------------------------------
// On death event
//
void Projectile::OnDeath()
{
	Entity::OnDeath();
}


//-----------------------------------------------------------------------------------------------
// On spawn event
//
void Projectile::OnSpawn()
{
	Entity::OnSpawn();

	m_stopwatch->SetInterval(m_definition->m_projectileLifetime);
}
