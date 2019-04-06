/************************************************************************/
/* File: Entity.cpp
/* Author: Andrew Chase
/* Date: April 1st 2019
/* Description: Implementation of the Entity class
/************************************************************************/
#include "Game/Entity/Entity.hpp"
#include "Game/Framework/Game.hpp"
#include "Game/Framework/GameCamera.hpp"
#include "Engine/Assets/AssetDB.hpp"
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


//-----------------------------------------------------------------------------------------------
// Applies the net force/impulses/acceleration/velocity to the entity
//
void Entity::ApplyPhysicsStep()
{
	float deltaSeconds = Game::GetDeltaTime();

	// Apply Friction/Air Drag
	ApplyHorizontalFrictionOrAirDrag();
	if (m_physicsMode != PHYSICS_MODE_WALKING)
	{
		ApplyVerticalAirDrag();
	}

	// Apply Force
	Vector3 accelerationFromForce = (m_force / m_mass);
	m_force = Vector3::ZERO;

	// Apply Impulse
	Vector3 deltaVelocityFromImpulse = (m_impulse / m_mass);
	m_velocity += deltaVelocityFromImpulse;
	m_impulse = Vector3::ZERO;

	// Apply Acceleration
	m_velocity += accelerationFromForce * deltaSeconds;

	// Apply Gravity
	if (m_physicsMode == PHYSICS_MODE_WALKING)
	{
		m_velocity += (Vector3::MINUS_Z_AXIS * ENTITY_GRAVITY_ACCELERATION) * deltaSeconds;
	}

	// Apply Velocity
	m_transform.position += m_velocity * deltaSeconds;

	// Reset Flags
	m_isOnGround = false;
}


//-----------------------------------------------------------------------------------------------
// Render
//
void Entity::Render() const
{
	// Don't render if we're possessed by the camera and in first person
	GameCamera* camera = Game::GetGameCamera();
	if (camera->IsAttachedToEntity(this) && camera->GetCameraMode() == CAMERA_MODE_ATTACHED_FIRST_PERSON)
	{
		return;
	}

	Renderer* renderer = Renderer::GetInstance();
	renderer->SetGLLineWidth(2.0f);
	
	AABB3 worldBounds = m_localPhysicsBounds.GetTranslated(m_transform.position);
	Vector3 absoluteCenter = worldBounds.GetCenter();
	Vector3 dimensions = worldBounds.GetDimensions();

	renderer->DrawWireCube(absoluteCenter, dimensions, Rgba::PURPLE);

	// Draw a point for the local origin
	renderer->DrawPoint(m_transform.position, Rgba::PURPLE, m_localPhysicsBounds.GetDimensions().x * 0.25f);
	renderer->SetGLLineWidth(1.0f);

	// Draw the velocity line
	renderer->Draw3DLine(m_transform.position, Rgba::YELLOW, m_transform.position + m_velocity, Rgba::YELLOW, 2.0f, AssetDB::GetSharedMaterial("Default_Opaque"));
	renderer->Draw3DLine(m_transform.position, Rgba::YELLOW, m_transform.position + m_velocity, Rgba::YELLOW, 2.0f, AssetDB::GetSharedMaterial("X_Ray"));
}


//-----------------------------------------------------------------------------------------------
// Calculates (and adds to the net force) the force that will accelerate the character in the given
// direction. Does extra math to prevent the entity from moving faster than its move speed
//
void Entity::MoveSelfHorizontal(const Vector2& directionToMove)
{
	float currentXYSpeed = m_velocity.xy().GetLength();
	float deltaSeconds = Game::GetDeltaTime();

	Vector2 finalXYVelocityIfFullAccelerationApplied = (m_velocity.xy() + (m_moveAcceleration * deltaSeconds) * directionToMove);

	Vector2 finalXYVelocityDirection = finalXYVelocityIfFullAccelerationApplied;
	float XYSpeedIfFullAccelerationApplied = finalXYVelocityDirection.NormalizeAndGetLength();
	float finalXYSpeed = XYSpeedIfFullAccelerationApplied;

	float maxXYMoveSpeed = (m_physicsMode == PHYSICS_MODE_WALKING ? m_maxXYWalkSpeed : m_maxXYFlySpeed);

	// Clamp to avoid accelerating over the move speed
	if (currentXYSpeed > maxXYMoveSpeed)
	{
		finalXYSpeed = ClampFloat(XYSpeedIfFullAccelerationApplied, 0.f, currentXYSpeed); // Don't go over, will maintain it without friction
	}
	else
	{
		finalXYSpeed = ClampFloat(XYSpeedIfFullAccelerationApplied, 0.f, maxXYMoveSpeed); // Approach max speed
	}

	// Create the final velocity
	Vector2 finalXYVelocity = finalXYVelocityDirection * finalXYSpeed;
	Vector3 finalVelocity = Vector3(finalXYVelocity.x, finalXYVelocity.y, m_velocity.z);

	// Determine the change in velocity
	Vector3 deltaVelocity = finalVelocity - m_velocity;

	// Find the force necessary to cause this delta
	Vector3 acceleration = deltaVelocity / deltaSeconds;
	Vector3 force = acceleration * m_mass;

	// Apply it!
	AddForce(force);
}


//-----------------------------------------------------------------------------------------------
// Moves the entity up or down using a force
// *NOT* to be used for jumping - only for flying
//
void Entity::MoveSelfVertical(const float directionToMove)
{
	float currentZSpeed = AbsoluteValue(m_velocity.z);
	float deltaSeconds = Game::GetDeltaTime();

	float zVelocityIfFullAccelerationApplied = (m_velocity.z + (m_moveAcceleration * deltaSeconds) * directionToMove);
	float finalResultingDirection = (zVelocityIfFullAccelerationApplied > 0.f ? 1.0f : -1.0f);
	float zSpeedIfFullAccelerationApplied = AbsoluteValue(zVelocityIfFullAccelerationApplied);
	float finalZSpeed = zSpeedIfFullAccelerationApplied;

	// Clamp to avoid accelerating over the move speed
	if (zSpeedIfFullAccelerationApplied > m_maxZMoveSpeed)
	{
		finalZSpeed = ClampFloat(zSpeedIfFullAccelerationApplied, 0.f, currentZSpeed); // Don't go over, will maintain it without friction
	}
	else
	{
		finalZSpeed = ClampFloat(zSpeedIfFullAccelerationApplied, 0.f, m_maxZMoveSpeed); // Approach max speed
	}

	// Create the final velocity
	float finalZVelocity = finalZSpeed * finalResultingDirection;

	// Determine the change in velocity
	float deltaZVelocity = finalZVelocity - m_velocity.z;

	// Find the force necessary to cause this delta
	float zAcceleration = deltaZVelocity / deltaSeconds;
	float zForce = zAcceleration * m_mass;

	// Apply it!
	AddForce(Vector3(0.f, 0.f, zForce));
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

	// 2.f * (jumpheight + g/8) = v0y 
	float initialYVelocity = 2.f * (m_jumpHeight + ENTITY_GRAVITY_ACCELERATION * 0.125f);

	float zImpulse = initialYVelocity * m_mass;
	AddImpulse(Vector3(0.f, 0.f, zImpulse));
}


//-----------------------------------------------------------------------------------------------
// Applies friction/Air drag based on the entity's state
//
void Entity::ApplyHorizontalFrictionOrAirDrag()
{
	// Friction is a force - only apply it if we're moving
	if (m_velocity.xy().GetLengthSquared() > 0.f)
	{
		// Different values of friction depending on ground or in air
		float decelerationPerSecond = ENTITY_GROUND_FRICTION_DECELERATION;
		if (!m_isOnGround)
		{
			decelerationPerSecond = ENTITY_AIR_DRAG_DECELERATION;
		}

		float deltaSeconds = Game::GetDeltaTime();
		float speedLostThisFrame = decelerationPerSecond * deltaSeconds;

		float speed = m_velocity.xy().GetLength();
		speedLostThisFrame = ClampFloat(speedLostThisFrame, 0.f, speed);
		
		Vector3 decelerationDirection = -1.0f * m_velocity.GetNormalized();
		decelerationDirection.z = 0.f;

		Vector3 finalDeceleration = decelerationDirection * (speedLostThisFrame / deltaSeconds);

		Vector3 finalForce = finalDeceleration * m_mass;

		AddForce(finalForce);
	}
}


//-----------------------------------------------------------------------------------------------
// Applies air drag in the z directions
// Should only be used with flying/no clip for a softer stop
//
void Entity::ApplyVerticalAirDrag()
{
	// Friction is a force - only apply it if we're moving
	if (m_velocity.z != 0.f)
	{
		float deltaSeconds = Game::GetDeltaTime();
		float zSpeedLostThisFrame = ENTITY_AIR_DRAG_DECELERATION * deltaSeconds;

		float zSpeed = AbsoluteValue(m_velocity.z);
		zSpeedLostThisFrame = ClampFloat(zSpeedLostThisFrame, 0.f, zSpeed);

		float zDecelerationDirection = (m_velocity.z > 0.f ? -1.f : 1.f);

		float finalZDeceleration = zDecelerationDirection * (zSpeedLostThisFrame / deltaSeconds);
		float finalZForce = finalZDeceleration * m_mass;

		AddForce(Vector3(0.f, 0.f, finalZForce));
	}
}


//-----------------------------------------------------------------------------------------------
// Returns the entity's physics bounds in world coordinates, based on its position
//
AABB3 Entity::GetWorldPhysicsBounds() const
{
	return m_localPhysicsBounds.GetTranslated(m_transform.position);
}
