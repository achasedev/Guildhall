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
// Called when the entity takes damage, for custom behavior
//
void DynamicEntity::OnDamageTaken(int damageAmount)
{
	Entity::OnDamageTaken(damageAmount);
}


//-----------------------------------------------------------------------------------------------
// Called when the entity's health is <= 0
//
void DynamicEntity::OnDeath()
{
	Entity::OnDeath();
}


//-----------------------------------------------------------------------------------------------
// Called when the entity is spawned into the world
//
void DynamicEntity::OnSpawn()
{
	Entity::OnSpawn();
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


//-----------------------------------------------------------------------------------------------
// Adds the given velocity immediately
//
void DynamicEntity::AddVelocity(const Vector3& velocity)
{
	m_velocity += velocity;
}


//-----------------------------------------------------------------------------------------------
// Sets the force to the given force
//
void DynamicEntity::SetForce(const Vector3& force)
{
	m_force = force;
}


//-----------------------------------------------------------------------------------------------
// Sets the velocity to the given velocity
//
void DynamicEntity::SetVelocity(const Vector3& velocity)
{
	m_velocity = velocity;
}


#include "Engine/Core/DeveloperConsole/DevConsole.hpp"
//-----------------------------------------------------------------------------------------------
// Performs the forward Euler physics step on the entity
//
void DynamicEntity::ApplyPhysicsStep()
{
	float deltaTime = Game::GetDeltaTime();

	if (m_affectedByGravity)
	{
		m_force += Vector3::DIRECTION_DOWN * m_mass * GRAVITY_MAGNITUDE;
	}

	// Apply force, clamping
	Vector3 acceleration = (m_force * m_inverseMass);
	float currAcceleration = acceleration.NormalizeAndGetLength();
	currAcceleration = ClampFloat(currAcceleration, 0.f, m_maxAcceleration);

	acceleration *= currAcceleration;

	// Apply acceleration
	m_velocity += (acceleration * deltaTime);

	float currSpeed = m_velocity.NormalizeAndGetLength();
	currSpeed = ClampFloat(currSpeed, 0.f, m_maxSpeed);
	m_velocity *= currSpeed;

	// Apply velocity
	m_position += (m_velocity * deltaTime);

	// Zero out force
	m_force = Vector3::ZERO;
}
