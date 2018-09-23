/************************************************************************/
/* File: DynamicEntity.cpp
/* Author: Andrew Chase
/* Date: September 22nd, 2017
/* Description: Implementation of the DynamicEntity class
/************************************************************************/
#include "Game/Entity/DynamicEntity.hpp"
#include "Game/Framework/Game.hpp"
#include "Engine/Math/MathUtils.hpp"

// Default acceleration due to gravity
#define GRAVITY_MAGNITUDE (9.81f);


//-----------------------------------------------------------------------------------------------
// Constructor
//
DynamicEntity::DynamicEntity()
	: Entity(ENTITY_TYPE_DYNAMIC)
{
}


//-----------------------------------------------------------------------------------------------
// Destructor
//
DynamicEntity::~DynamicEntity()
{
}


//-----------------------------------------------------------------------------------------------
// Update
//
void DynamicEntity::Update()
{
	Entity::Update();
}


//-----------------------------------------------------------------------------------------------
// Callback for when collision is detected with an entity
//
void DynamicEntity::OnCollision(Entity* other)
{
	Entity::OnCollision(other);
}


//-----------------------------------------------------------------------------------------------
// Adds the correction to the entity's position to fix a collision
//
void DynamicEntity::AddCollisionCorrection(const Vector3& correction)
{
	m_position += correction;
}


//-----------------------------------------------------------------------------------------------
// Returns the mass of the entity
//
float DynamicEntity::GetMass() const
{
	return m_mass;
}


//-----------------------------------------------------------------------------------------------
// Returns one over the mass of the entity
//
float DynamicEntity::GetInverseMass() const
{
	return m_inverseMass;
}


//-----------------------------------------------------------------------------------------------
// Adds the given force, to be applied during the physics step
//
void DynamicEntity::AddForce(const Vector3& force)
{
	m_force += force;
}

#include "Engine/Core/DeveloperConsole/DevConsole.hpp"
//-----------------------------------------------------------------------------------------------
// Performs the forward Euler physics step on the entity
//
void DynamicEntity::ApplyPhysicsStep()
{
	if (m_affectedByGravity)
	{
		m_force += Vector3::DIRECTION_DOWN * m_mass * GRAVITY_MAGNITUDE;
	}

	// Apply force
	m_acceleration = (m_force * m_inverseMass);
	float currAcceleration = m_acceleration.GetLength();
	//currAcceleration -= 20.f * m_inverseMass * Game::GetDeltaTime();
	//currAcceleration = ClampFloat(currAcceleration, 0.f, 1000.f);
	//m_acceleration *= currAcceleration;

	// Apply acceleration
	float deltaTime = Game::GetDeltaTime();

	m_velocity += (m_acceleration * deltaTime);

	float currSpeed = m_velocity.NormalizeAndGetLength();
	float newSpeed = currSpeed;
	if (currAcceleration == 0.f)
	{
		newSpeed -= 10.f * m_inverseMass * Game::GetDeltaTime();
	}

	newSpeed = ClampFloat(newSpeed, 0.f, 100.f);
	m_velocity *= newSpeed;

	// Apply velocity
	m_position += (m_velocity * deltaTime);

	ConsolePrintf("Speed: %f", newSpeed);

	m_force = Vector3::ZERO;
}
