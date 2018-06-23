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
	~Bullet();

	virtual void Update(float deltaTime) override;

	unsigned int GetDamageAmount() const;


private:
	//-----Private Data-----

	float m_timeToLive;
	unsigned int m_damageAmount = 2; 

	static const float BULLET_SPEED;
	static const float BULLET_LIFETIME;

};
