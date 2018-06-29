/************************************************************************/
/* File: Bullet.hpp
/* Author: Andrew Chase
/* Date: June 18th, 2018
/* Description: Class to represent a fired projectile
/************************************************************************/
#pragma once
#include "Game/Entity/GameEntity.hpp"

class Bullet : public GameEntity
{
public:
	//-----Public Methods-----

	Bullet(const Vector3& position, const Quaternion& orientation, unsigned int team);
	virtual ~Bullet();

	virtual void Update(float deltaTime) override;

	unsigned int GetDamageAmount() const;


protected:
	//-----Protected Data-----

	float m_timeToLive;
	unsigned int m_damageAmount = 2; 

	float m_speed;

	static const float BULLET_DEFAULT_SPEED;
	static const float BULLET_DEFAULT_LIFETIME;

};
