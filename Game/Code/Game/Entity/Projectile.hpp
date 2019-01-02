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

	Projectile(const EntityDefinition* definition, eEntityTeam team);

	virtual void Update() override;
	virtual const VoxelSprite* GetVoxelSprite() const override;

	// Events
	virtual void OnEntityCollision(Entity* other) override;
	virtual void OnGroundCollision() override;
	virtual void OnDamageTaken(int damageAmount) override;
	virtual void OnDeath() override;
	virtual void OnSpawn() override;


private:
	//-----Private Data-----

	bool		m_applyPhysics = true;
	Stopwatch*	m_stopwatch = nullptr;

};
