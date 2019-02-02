/************************************************************************/
/* File: VoxelEmitter.hpp
/* Author: Andrew Chase
/* Date: November 6th 2018
/* Description: Class to represent a voxel particle emitter
/************************************************************************/
#pragma once
#include "Engine/Core/Time/Stopwatch.hpp"
#include "Engine/Math/Vector3.hpp"

class VoxelEmitter
{
public:
	//-----Public Methods-----

	VoxelEmitter(float spawnRate, float particleLifetime, const Vector3& position, const Vector3& initialVelocity, float spread);

	void Update();
	
	
private:
	//-----Private Data-----
	
	float m_spawnInterval = 0.f;
	float m_spread = 0.f;
	float m_particleLifetime = 1.0f;
	Vector3 m_spawnPosition = Vector3::ZERO;
	Vector3 m_initialParticleVelocity = Vector3::Y_AXIS;
	Stopwatch m_stopwatch;
	
	bool m_shouldEmit = true;

};
