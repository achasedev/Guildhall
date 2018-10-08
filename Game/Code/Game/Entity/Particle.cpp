/************************************************************************/
/* File: Particle.cpp
/* Author: Andrew Chase
/* Date: October 8th, 2018
/* Description: Implementation of the Particle class
/************************************************************************/
#include "Game/Framework/Game.hpp"
#include "Game/Entity/Particle.hpp"
#include "Game/Entity/PhysicsComponent.hpp"


//-----------------------------------------------------------------------------------------------
// Constructor - assembles the texture 
//
Particle::Particle(const Rgba& color, float lifetime, const Vector3& position, const Vector3& initialVelocity)
	: Entity(EntityDefinition::GetDefinition("Particle"))
{
	m_particleTexture = new VoxelTexture();
	m_particleTexture->CreateFromColorStream(&color, IntVector3(1, 1, 1));

	m_position = position;
	m_lifetime = lifetime;

	m_physicsComponent->SetVelocity(initialVelocity);
	m_stopwatch.SetClock(Game::GetGameClock());
}


//-----------------------------------------------------------------------------------------------
// Destructor
//
Particle::~Particle()
{
	if (m_particleTexture != nullptr)
	{
		delete m_particleTexture;
		m_particleTexture = nullptr;
	}
}


//-----------------------------------------------------------------------------------------------
// Update loop
//
void Particle::Update()
{
	if (m_stopwatch.HasIntervalElapsed())
	{
		m_isMarkedForDelete = true;
	}
}


//-----------------------------------------------------------------------------------------------
// Override for drawing - particles don't use an animator
//
const VoxelTexture* Particle::GetTextureForOrientation() const
{
	return m_particleTexture;
}


//-----------------------------------------------------------------------------------------------
// Starts the particle's lifetime timer
//
void Particle::OnSpawn()
{
	m_stopwatch.SetInterval(m_lifetime);
}
