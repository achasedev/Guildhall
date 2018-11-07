/************************************************************************/
/* File: VoxelEmitter.cpp
/* Author: Andrew Chase
/* Date: November 6th 2018
/* Description: Implementation of the VoxelEmitter class
/************************************************************************/
#include "Game/Framework/Game.hpp"
#include "Game/Framework/World.hpp"
#include "Game/Entity/Particle.hpp"
#include "Game/Animation/VoxelEmitter.hpp"
#include "Engine/Math/MathUtils.hpp"


//-----------------------------------------------------------------------------------------------
// Constructor
//
VoxelEmitter::VoxelEmitter(float spawnRate, float particleLifetime, const Vector3& position, const Vector3& initialVelocity, float spread)
	: m_spawnInterval(1.f / spawnRate)
	, m_initialParticleVelocity(initialVelocity)
	, m_spread(spread)
	, m_particleLifetime(particleLifetime)
	, m_spawnPosition(position)
{
	m_stopwatch.SetInterval(m_spawnInterval);
}


//-----------------------------------------------------------------------------------------------
// Update
//
void VoxelEmitter::Update()
{
	if (m_shouldEmit)
	{
		int numToSpawn = m_stopwatch.DecrementByIntervalAll();

		for (int i = 0; i < numToSpawn; ++i)
		{
			Vector3 spread = Vector3(GetRandomFloatInRange(-m_spread, m_spread), GetRandomFloatInRange(-m_spread, m_spread), GetRandomFloatInRange(-m_spread, m_spread));
			Particle* particle = new Particle(Rgba::GetRandomColor(), m_particleLifetime, m_spawnPosition, m_initialParticleVelocity + spread);
			Game::GetWorld()->AddParticle(particle);
		}
	}
}
