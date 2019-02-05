/************************************************************************/
/* File: Particle.hpp
/* Author: Andrew Chase
/* Date: September 29th 2018
/* Description: Class to represent a single voxel particle
/************************************************************************/
#pragma once
#include "Game/Entity/Entity.hpp"
#include "Engine/Core/Time/Stopwatch.hpp"

class Rgba;

class Particle : public Entity
{
public:
	//-----Public Methods-----

	Particle(const Rgba& color, float lifetime, const Vector3& position, const Vector3& initialVelocity, bool attachToGround = false, bool fillsHoles = false);
	~Particle();
	
	virtual void					Update() override;
	virtual void					OnSpawn() override;
	virtual void					OnGroundCollision() override;
	
	
private:
	//-----Private Data-----
	
	Stopwatch m_stopwatch;
	float m_lifetime = DEFAULT_LIFETIME;
	float m_initialSpeed = DEFAULT_SPEED;
	bool m_attachToGround = false;
	bool m_fillsHoles = false;

	static constexpr float DEFAULT_SPEED = 10.f;
	static constexpr float DEFAULT_LIFETIME = 30.0f;

};
