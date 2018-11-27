/************************************************************************/
/* File: PhysicsComponent.cpp
/* Author: Andrew Chase
/* Date: October 8th, 2018
/* Description: Implementation of the PhysicsComponent class
/************************************************************************/
#include "Game/Entity/Components/PhysicsComponent.hpp"
#include "Game/Framework/Game.hpp"
#include "Engine/Math/MathUtils.hpp"

// Default acceleration due to gravity
#define GRAVITY_MAGNITUDE (200.f);


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


//-----------------------------------------------------------------------------------------------
// Adds the given force to the component
//
void PhysicsComponent::AddForce(const Vector3& force)
{
	m_force += force;
}


//-----------------------------------------------------------------------------------------------
// Adds the given impulse to the component
//
void PhysicsComponent::AddImpulse(const Vector3& impulse)
{
	m_impulse += impulse;
}


//-----------------------------------------------------------------------------------------------
// Adds the given velocity to the component
//
void PhysicsComponent::AddVelocity(const Vector3& velocity)
{
	m_velocity += velocity;
}


//-----------------------------------------------------------------------------------------------
// Sets the force of the component to the given force
//
void PhysicsComponent::SetForce(const Vector3& force)
{
	m_force = force;
}


//-----------------------------------------------------------------------------------------------
// Sets the velocity of the component to the given velocity
//
void PhysicsComponent::SetVelocity(const Vector3& velocity)
{
	m_velocity = velocity;
}


//-----------------------------------------------------------------------------------------------
// Sets the x-component of the velocity to 0; used during collision correction
//
void PhysicsComponent::ZeroXVelocity()
{
	m_velocity.x = 0.f;
}


//-----------------------------------------------------------------------------------------------
// Sets the y-component of the velocity to 0; used during collision correction
//
void PhysicsComponent::ZeroYVelocity()
{
	m_velocity.y = 0.f;
	m_owningEntity->SetIsGrounded(true);
}


//-----------------------------------------------------------------------------------------------
// Sets the z-component of the velocity to 0; used during collision correction
//
void PhysicsComponent::ZeroZVelocity()
{
	m_velocity.z = 0.f;
}


//-----------------------------------------------------------------------------------------------
// Sets whether this component should apply gravity
//
void PhysicsComponent::SetGravity(bool hasGravity)
{
	m_affectedByGravity = hasGravity;
}


//-----------------------------------------------------------------------------------------------
// Immediately stops all movement on the component
//
void PhysicsComponent::StopAllMovement()
{
	m_velocity = Vector3::ZERO;
	m_force = Vector3::ZERO;
	m_impulse = Vector3::ZERO;
}


//-----------------------------------------------------------------------------------------------
// Applies the forward Euler computation to the velocity and position of the owning entity
//
void PhysicsComponent::ApplyPhysicsStep()
{
	// Apply impulse, no delta time applied
	Vector3 deltaVelocityFromImpulse = (m_impulse * m_owningEntity->GetInverseMass());
	m_velocity += deltaVelocityFromImpulse;

	// Apply force
	if (m_affectedByGravity)
	{
		m_force += Vector3::DIRECTION_DOWN * m_owningEntity->GetMass() * GRAVITY_MAGNITUDE;
		m_owningEntity->SetIsGrounded(false); // Collision detection will set this to true if we're falling through something
	}

	Vector3 acceleration = (m_force * m_owningEntity->GetInverseMass());
	float currAcceleration = acceleration.NormalizeAndGetLength();
	currAcceleration = ClampFloat(currAcceleration, 0.f, m_maxAcceleration);
	acceleration *= currAcceleration;

	float deltaTime =Game::GetDeltaTime();
	m_velocity += (acceleration * deltaTime);

	// Clamp velocity - component wise
	m_velocity = ClampVector3(m_velocity, -m_maxSpeed, m_maxSpeed);

	// Apply velocity
	m_owningEntity->AddPositionOffset(m_velocity * deltaTime);

	// Zero out force and impulse
	m_force = Vector3::ZERO;
	m_impulse = Vector3::ZERO;
}


//-----------------------------------------------------------------------------------------------
// Returns the current velocity of the component
//
Vector3 PhysicsComponent::GetVelocity() const
{
	return m_velocity;
}
