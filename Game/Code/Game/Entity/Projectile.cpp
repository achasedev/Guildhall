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
// Constructor
//
Projectile::Projectile()
{
	m_stopwatch = new Stopwatch(Game::GetGameClock());
	m_collisionDef = CollisionDefinition_t(COLLISION_TYPE_BOX, 1.f, 1.f, 1.f);

	SetupVoxelTextures("Data/3DTextures/TestProjectile.qef");
}


//-----------------------------------------------------------------------------------------------
// Update
//
void Projectile::Update()
{
	DynamicEntity::Update();

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
	DynamicEntity::OnCollision(other);
}


//-----------------------------------------------------------------------------------------------
// On damage taken event
//
void Projectile::OnDamageTaken(int damageAmount)
{
	DynamicEntity::OnDamageTaken(damageAmount);
}


//-----------------------------------------------------------------------------------------------
// On death event
//
void Projectile::OnDeath()
{
	DynamicEntity::OnDeath();
}


//-----------------------------------------------------------------------------------------------
// On spawn event
//
void Projectile::OnSpawn()
{
	DynamicEntity::OnSpawn();

	m_stopwatch->SetInterval(m_lifetime);
}
