/************************************************************************/
/* File: Projectile.hpp
/* Author: Andrew Chase
/* Date: November 18th, 2017
/* Bugs: None
/* Description: Class to represent a spawned projectile in the game
/************************************************************************/
#pragma once
#include "Game/Entity.hpp"

class ProjectileDefinition;


class Projectile : public Entity
{
public:
	//-----Public Methods-----

	Projectile(const Vector2& position, float orientation, const ProjectileDefinition* definition, Map* entityMap, const std::string& name, const std::string& faction);

	virtual void Update(float deltaTime) override;
	virtual void Render() const override;

	// Accessors/Producers
	virtual std::string		GetFaction() const override;


private:
	//-----Private Data-----

	std::string m_faction;		// The faction of the Actor that fired this projectile
	float m_maxAge = -1.f;		// Age that forces this entity to die

	const ProjectileDefinition* m_projectileDefinition;
};
