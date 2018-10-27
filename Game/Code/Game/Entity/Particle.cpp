/************************************************************************/
/* File: Particle.cpp
/* Author: Andrew Chase
/* Date: October 8th, 2018
/* Description: Implementation of the Particle class
/************************************************************************/
#include "Game/Framework/Game.hpp"
#include "Game/Entity/Particle.hpp"
#include "Game/Animation/VoxelAnimator.hpp"
#include "Game/Entity/Components/PhysicsComponent.hpp"
#include "Engine/Core/Utility/ErrorWarningAssert.hpp"

//-----------------------------------------------------------------------------------------------
// Constructor - assembles the texture 
//
Particle::Particle(const Rgba& color, float lifetime, const Vector3& position, const Vector3& initialVelocity)
	: Entity(EntityDefinition::GetDefinition("Particle"))
{
	ASSERT_OR_DIE(m_defaultTexture == nullptr, "Error: Particle definition has a default texture!");

	m_defaultTexture = new VoxelTexture();
	m_defaultTexture->CreateFromColorStream(&color, IntVector3(1, 1, 1));

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
// Sets the flag used to check whether the particle should continue simulating physics
//
void Particle::SetApplyPhysics(bool newState)
{
	m_applyPhysics = newState;
}


//-----------------------------------------------------------------------------------------------
// Returns whether this particle should have physics applied to it
//
bool Particle::ShouldApplyPhysics() const
{
	return m_applyPhysics;
}


//-----------------------------------------------------------------------------------------------
// Starts the particle's lifetime timer
//
void Particle::OnSpawn()
{
	m_stopwatch.SetInterval(m_lifetime);
}
