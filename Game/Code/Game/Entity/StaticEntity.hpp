/************************************************************************/
/* File: StaticEntity.hpp
/* Author: Andrew Chase
/* Date: September 28th 2018
/* Description: Class to represent an object with no physics logic
/************************************************************************/
#pragma once
#include "Game/Entity/Entity.hpp"

// Ensure any collision checks we make don't move this object
// (Any collider 100x+ the mass of another is automatically not moved)
#define REALLY_BIG_MASS (1000000)

class StaticEntity : public Entity
{
public:
	//-----Public Methods-----

	StaticEntity();
	~StaticEntity();

	// Core
	virtual void Update() override;

	// Collision
	virtual void OnCollision(Entity* other) override;
	virtual void OnDamageTaken(int damageAmount) override;
	virtual void OnDeath() override;
	virtual void OnSpawn() override;


protected:
	//-----Protected Data-----


};
