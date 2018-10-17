/************************************************************************/
/* File: Projectile.hpp
/* Author: Andrew Chase
/* Date: September 28th 2018
/* Description: Class to represent a flying projectile
/************************************************************************/
#pragma once
#include "Game/Entity/Entity.hpp"

class Stopwatch;

class Projectile : public Entity
{
public:
	//-----Public Methods-----

	Projectile(const EntityDefinition* definition);

	virtual void Update() override;

	// Events
	virtual void OnCollision(Entity* other) override;
	virtual void OnDamageTaken(int damageAmount) override;
	virtual void OnDeath() override;
	virtual void OnSpawn() override;


private:
	//-----Private Data-----

	int			m_damage	= DEFAULT_DAMAGE;
	float		m_speed		= 10.f;
	float		m_lifetime	= DEFAULT_LIFETIME;
	bool		m_applyPhysics = true;
	Stopwatch*	m_stopwatch = nullptr;

	static constexpr int	DEFAULT_DAMAGE = 1;
	static constexpr float	DEFAULT_LIFETIME = 1.0f;
	static constexpr float	DEFAULT_SPEED = 100.f;

};
