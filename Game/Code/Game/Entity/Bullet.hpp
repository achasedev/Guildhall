/************************************************************************/
/* File: Bullet.hpp
/* Author: Andrew Chase
/* Date: June 18th, 2018
/* Description: Class to represent a fired projectile
/************************************************************************/
#pragma once
#include "Engine/Core/GameObject.hpp"

class Bullet : public GameObject
{
public:
	//-----Public Methods-----

	Bullet(const Vector3& position, const Quaternion& orientation);
	~Bullet();

	virtual void Update(float deltaTime) override;


private:
	//-----Private Data-----

	float m_timeToLive;

	static const float BULLET_SPEED;
	static const float BULLET_LIFETIME;

};
