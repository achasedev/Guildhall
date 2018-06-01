/************************************************************************/
/* Project: Incursion
/* File: Bullet.hpp
/* Author: Andrew Chase
/* Date: October 7th, 2017
/* Bugs: None
/* Description: Class to represent a generic bullet projectile
/************************************************************************/
#pragma once
#include "Game/Entity.hpp"

class Bullet : public Entity
{
public:
	//-----Public Methods-----
	Bullet(Vector2 position, float orientation, Faction entityFaction, Map* currentMap);
	~Bullet();

	virtual void Update(float deltaTime) override;
	virtual void Render() const override;

	Vector2 GetNextPosition() const;

public:

private:

private:
	//-----Private Data-----

	float m_speed;
	Vector2 m_velocity;

	static const float DEFAULT_BULLET_SPEED;			// Default bullet speed
	static const float BULLET_OUTER_RADIUS;		// Bullet cosmetic radius
	static const float BULLET_INNER_RADIUS;		// Bullet physics radius
};