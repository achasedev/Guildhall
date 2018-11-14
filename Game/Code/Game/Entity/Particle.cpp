/************************************************************************/
/* File: Particle.cpp
/* Author: Andrew Chase
/* Date: October 8th, 2018
/* Description: Implementation of the Particle class
/************************************************************************/
#include "Game/Framework/Game.hpp"
#include "Game/Framework/World.hpp"
#include "Game/Entity/Particle.hpp"
#include "Game/Animation/VoxelAnimator.hpp"
#include "Game/Entity/Components/PhysicsComponent.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Utility/HeatMap.hpp"
#include "Engine/Core/Utility/ErrorWarningAssert.hpp"

//-----------------------------------------------------------------------------------------------
// Constructor - assembles the texture 
//
Particle::Particle(const Rgba& color, float lifetime, const Vector3& position, const Vector3& initialVelocity, bool attachToGround /*= false*/)
	: Entity(EntityDefinition::GetDefinition("Particle"))
{
	ASSERT_OR_DIE(m_defaultTexture == nullptr, "Error: Particle definition has a default texture!");

	m_defaultTexture = new VoxelTexture();
	m_defaultTexture->CreateFromColorStream(&color, IntVector3(1, 1, 1), true);

	m_position = position;
	m_lifetime = lifetime;
	m_attachToGround = attachToGround;

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
	if (Game::GetWorld()->IsEntityOnGround(this))
	{
		m_physicsEnabled = false;
	}

	if (m_stopwatch.HasIntervalElapsed())
	{
		m_isMarkedForDelete = true;
	}
}


//-----------------------------------------------------------------------------------------------
// Starts the particle's lifetime timer
//
void Particle::OnSpawn()
{
	m_stopwatch.SetInterval(m_lifetime);
}


//-----------------------------------------------------------------------------------------------
// If flagged to attach, have the particle become part of the ground
//
void Particle::OnGroundCollision()
{
	if (m_attachToGround)
	{
		IntVector3 coordPosition = GetCoordinatePosition();
		World* world = Game::GetWorld();
		const HeatMap* heightMap = world->GetHeightMap();

		if (heightMap->AreCoordsValid(coordPosition.xz()))
		{
			int mapHeight = (int) heightMap->GetHeat(coordPosition.xz());

			int finalHeight = MaxInt(mapHeight, coordPosition.y + 1);
			world->SetTerrainHeightAtCoord(coordPosition, finalHeight);

			m_isMarkedForDelete = true;
		}
	}
}
