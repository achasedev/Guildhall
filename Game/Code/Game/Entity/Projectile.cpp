/************************************************************************/
/* File: Projectile.cpp
/* Author: Andrew Chase
/* Date: September 28th 2018
/* Description: Implementation of the projectile class
/************************************************************************/
#include "Game/Framework/Game.hpp"
#include "Game/Entity/Projectile.hpp"
#include "Engine/Core/Time/Stopwatch.hpp"


//-----------------------------------------------------------------------------------------------
// Constructor from a definition
//
Projectile::Projectile(const EntityDefinition* definition)
	: Entity(definition)
{
	m_stopwatch = new Stopwatch(Game::GetGameClock());
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
// On Collision event
//
void Projectile::OnCollision(Entity* other)
{
	Entity::OnCollision(other);
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

	m_stopwatch->SetInterval(m_lifetime);
}
