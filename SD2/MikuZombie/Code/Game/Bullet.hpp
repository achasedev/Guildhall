/************************************************************************/
/* File: Bullet.hpp
/* Author: Andrew Chase
/* Date: May 9th, 2018
/* Description: Class to represent a bullet
/************************************************************************/
#pragma once
#include "Engine/Math/Transform.hpp"
#include "Engine/Core/GameObject.hpp"

class ParticleEmitter;
class Light;

class Bullet : public GameObject
{
public:
	//-----Public Methods-----

	Bullet(const Transform& travelDirection);
	~Bullet();

	virtual void Update(float deltaTime) override;


private:
	//-----Private Data-----

	ParticleEmitter* m_trailEmitter;
	Light* m_light;

	Vector3 m_velocity;
	float m_timeToLive;

	// Static constants
	static constexpr float BULLET_LIFETIME = 3.f;
	static constexpr float BULLET_SPEED = 60.f;
};
