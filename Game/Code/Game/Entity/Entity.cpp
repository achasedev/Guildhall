/************************************************************************/
/* File: Entity.cpp
/* Author: Andrew Chase
/* Date: April 1st 2019
/* Description: Implementation of the Entity class
/************************************************************************/
#include "Game/Entity/Entity.hpp"
#include "Game/Framework/Game.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Rendering/Core/Renderer.hpp"

// Static constants
const Vector3 Entity::ENTITY_DEFAULT_LOCAL_PHYSICS_BACK_LEFT_BOTTOM = Vector3(-0.5f * ENTITY_DEFAULT_PHYSICS_LENGTH_X, -0.5f * ENTITY_DEFAULT_PHYSICS_WIDTH_Y, 0.f);
const Vector3 Entity::ENTITY_DEFAULT_LOCAL_PHYSICS_FRONT_RIGHT_TOP = Vector3(0.5f * ENTITY_DEFAULT_PHYSICS_LENGTH_X, 0.5f * ENTITY_DEFAULT_PHYSICS_WIDTH_Y, ENTITY_DEFAULT_PHYSICS_HEIGHT_Z);


//-----------------------------------------------------------------------------------------------
// Update
//
void Entity::Update()
{
	float deltaSeconds = Game::GetDeltaTime();
	m_ageSeconds += deltaSeconds;
}


#include "Engine/Core/DeveloperConsole/DevConsole.hpp"
//-----------------------------------------------------------------------------------------------
// Applies the net force/impulses/acceleration/velocity to the entity
//
void Entity::ApplyPhysicsStep()
{
	float deltaSeconds = Game::GetDeltaTime();

	// Apply Friction/Air Drag
	ApplyFrictionOrAirDrag();

	// Apply Force
	ConsolePrintf("Friction Force: (%.4f, %.4f, %.4f)", m_force.x, m_force.y, m_force.z);
	Vector3 accelerationFromForce = (m_force / m_mass);
	m_acceleration += accelerationFromForce;
	m_force = Vector3::ZERO;

	// Apply Impulse
	Vector3 deltaVelocityFromImpulse = (m_impulse / m_mass);
	m_velocity += deltaVelocityFromImpulse;
	m_impulse = Vector3::ZERO;

	// Apply Acceleration
	m_velocity += m_acceleration * deltaSeconds;
	m_acceleration = Vector3::ZERO;
	

	// Apply Gravity
	if (m_physicsMode == PHYSICS_MODE_WALKING)
	{
		m_velocity += (Vector3::MINUS_Z_AXIS * ENTITY_GRAVITY_ACCELERATION) * deltaSeconds;
	}

	// Apply Velocity
	m_position += m_velocity * deltaSeconds;

	// Reset Flags
	m_isOnGround = false;
}


//-----------------------------------------------------------------------------------------------
// Render
//
void Entity::Render() const
{
	Renderer* renderer = Renderer::GetInstance();

	renderer->SetGLLineWidth(2.0f);
	
	AABB3 worldBounds = m_localPhysicsBounds.GetTranslated(m_position);
	Vector3 absoluteCenter = worldBounds.GetCenter();
	Vector3 dimensions = worldBounds.GetDimensions();

	renderer->DrawWireCube(absoluteCenter, dimensions, Rgba::PURPLE);
	renderer->SetGLLineWidth(1.0f);
}


//-----------------------------------------------------------------------------------------------
// Calculates (and adds to the net force) the force that will accelerate the character in the given
// direction. Does extra math to prevent the entity from moving faster than its move speed
//
void Entity::MoveSelf(const Vector3& directionToMove)
{
	float currentSpeed = m_velocity.GetLength();
	float deltaSeconds = Game::GetDeltaTime();

	Vector3 finalVelocityIfFullAccelerationApplied = (m_velocity + (m_moveAcceleration * deltaSeconds) * directionToMove);

	Vector3 finalVelocityDirection = finalVelocityIfFullAccelerationApplied;
	float speedIfFullAccelerationApplied = finalVelocityDirection.NormalizeAndGetLength();
	float finalSpeed = speedIfFullAccelerationApplied;

	// Clamp to avoid accelerating over the move speed
	if (currentSpeed > m_maxMoveSpeed)
	{
		finalSpeed = ClampFloat(speedIfFullAccelerationApplied, 0.f, currentSpeed); // Don't go over, will maintain it without friction
	}
	else
	{
		finalSpeed = ClampFloat(speedIfFullAccelerationApplied, 0.f, m_maxMoveSpeed); // Approach max speed
	}

	// Create the final velocity
	Vector3 finalVelocity = finalVelocityDirection * finalSpeed;

	// Determine the change in velocity
	Vector3 deltaVelocity = finalVelocity - m_velocity;

	// Find the force necessary to cause this delta
	Vector3 acceleration = deltaVelocity / deltaSeconds;
	Vector3 force = acceleration * m_mass;

	// Apply it!
	AddForce(force);
}


//-----------------------------------------------------------------------------------------------
// Calculates the force necessary to make the entity jump up to its jump height
// Only works if the entity is on the ground
//
void Entity::Jump()
{
	if (!m_isOnGround)
	{
		return;
	}

	// y = y0 + vyt -1/2gt2

	// 2.f * (jumpheight + g/8) = vy 
	float initialYVelocity = 2.f * (m_jumpHeight + ENTITY_GRAVITY_ACCELERATION * 0.125f);

	float zImpulse = initialYVelocity * m_mass;
	AddImpulse(Vector3(0.f, 0.f, zImpulse));
}


//-----------------------------------------------------------------------------------------------
// Applies friction/Air drag based on the entity's state
//
void Entity::ApplyFrictionOrAirDrag()
{
	// Friction is a force - only apply it if we're moving
	if (m_velocity.GetLengthSquared() > 0)
	{
		// Different values of friction depending on ground or in air
		float decelerationPerSecond = ENTITY_GROUND_FRICTION_DECELERATION;
		if (!m_isOnGround)
		{
			decelerationPerSecond = ENTITY_AIR_DRAG_DECELERATION;
		}

		float deltaSeconds = Game::GetDeltaTime();
		float speedLostThisFrame = decelerationPerSecond * deltaSeconds;

		float speed = m_velocity.GetLength();
		speedLostThisFrame = ClampFloat(speedLostThisFrame, 0.f, speed);
		
		Vector3 decelerationDirection = -1.0f * m_velocity.GetNormalized();
		Vector3 finalDeceleration = decelerationDirection * (speedLostThisFrame / deltaSeconds);

		Vector3 finalForce = finalDeceleration * m_mass;

		AddForce(finalForce);
	}
}


//-----------------------------------------------------------------------------------------------
// Returns the entity's physics bounds in world coordinates, based on its position
//
AABB3 Entity::GetWorldPhysicsBounds() const
{
	return m_localPhysicsBounds.GetTranslated(m_position);
}
