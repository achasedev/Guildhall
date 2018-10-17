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

	Particle(const Rgba& color, float lifetime, const Vector3& position, const Vector3& initialVelocity);
	~Particle();
	
	virtual void					Update() override;

	void SetApplyPhysics(bool newState);
	bool ShouldApplyPhysics() const;

	virtual const VoxelTexture*		GetTextureForOrientation() const override;
	virtual void					OnSpawn() override;
	
	
private:
	//-----Private Data-----
	
	VoxelTexture* m_particleTexture = nullptr;

	bool m_applyPhysics = true;

	Stopwatch m_stopwatch;
	float m_lifetime = DEFAULT_LIFETIME;
	float m_initialSpeed = DEFAULT_SPEED;

	static constexpr float DEFAULT_SPEED = 10.f;
	static constexpr float DEFAULT_LIFETIME = 30.0f;

};
