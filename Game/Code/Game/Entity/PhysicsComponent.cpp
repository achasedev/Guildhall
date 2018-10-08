/************************************************************************/
/* File: DynamicEntity.cpp
/* Author: Andrew Chase
/* Date: September 22nd, 2017
/* Description: Implementation of the DynamicEntity class
/************************************************************************/
#include "Game/Entity/PhysicsComponent.hpp"
#include "Game/Framework/Game.hpp"
#include "Engine/Math/MathUtils.hpp"

// Default acceleration due to gravity
#define GRAVITY_MAGNITUDE (100.f);


//-----------------------------------------------------------------------------------------------
// Constructor
//
PhysicsComponent::PhysicsComponent(Entity* entity)
	: m_owningEntity(entity)
{
	m_affectedByGravity = m_owningEntity->GetEntityDefinition()->HasGravity();
}


//-----------------------------------------------------------------------------------------------
// Destructor
//
PhysicsComponent::~PhysicsComponent()
{
}


void PhysicsComponent::AddForce(const Vector3& force)
{
	m_force += force;
}


void PhysicsComponent::AddImpulse(const Vector3& impulse)
{
	m_impulse += impulse;
}

void PhysicsComponent::AddVelocity(const Vector3& velocity)
{
	m_velocity += velocity;
}

void PhysicsComponent::SetForce(const Vector3& force)
{
	m_force = force;
}

void PhysicsComponent::SetVelocity(const Vector3& velocity)
{
	m_velocity = velocity;
}

void PhysicsComponent::ApplyPhysicsStep()
{
	// Apply impulse, no delta time applied
	Vector3 deltaVelocityFromImpulse = (m_impulse * m_owningEntity->GetInverseMass());
	m_velocity += deltaVelocityFromImpulse;

	// Apply force
	if (m_affectedByGravity)
	{
		m_force += Vector3::DIRECTION_DOWN * m_owningEntity->GetMass() * GRAVITY_MAGNITUDE;
	}

	Vector3 acceleration = (m_force * m_owningEntity->GetInverseMass());
	float currAcceleration = acceleration.NormalizeAndGetLength();
	currAcceleration = ClampFloat(currAcceleration, 0.f, m_maxAcceleration);
	acceleration *= currAcceleration;

	float deltaTime = Game::GetDeltaTime();
	m_velocity += (acceleration * deltaTime);

	// Clamp velocity - component wise
	m_velocity = ClampVector3(m_velocity, -m_maxSpeed, m_maxSpeed);

	// Apply velocity
	m_owningEntity->AddPositionOffset(m_velocity * deltaTime);

	// Zero out force and impulse
	m_force = Vector3::ZERO;
	m_impulse = Vector3::ZERO;
}

Vector3 PhysicsComponent::GetVelocity() const
{
	return m_velocity;
}

