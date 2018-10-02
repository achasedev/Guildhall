/************************************************************************/
/* File: DynamicEntity.cpp
/* Author: Andrew Chase
/* Date: September 22nd, 2017
/* Description: Implementation of the DynamicEntity class
/************************************************************************/
#include "Game/Entity/DynamicEntity.hpp"
#include "Game/Framework/Game.hpp"
#include "Engine/Math/MathUtils.hpp"

#include "Game/Animation/VoxelAnimationSet.hpp"
#include "Game/Animation/VoxelAnimator.hpp"
#include "Game/Animation/VoxelSprite.hpp"

// Default acceleration due to gravity
#define GRAVITY_MAGNITUDE (100.f);


//-----------------------------------------------------------------------------------------------
// Constructor
//
DynamicEntity::DynamicEntity()
	: Entity(ENTITY_TYPE_DYNAMIC)
{
	SetPosition(Vector3(GetRandomFloatInRange(10.f, 250.f), 4.f, GetRandomFloatInRange(10.f, 250.f)));
	m_collisionDef = CollisionDefinition_t(COLLISION_SHAPE_BOX, COLLISION_RESPONSE_FULL_CORRECTION, 4.f, 4.f, 8.f);
	m_animator = new VoxelAnimator(VoxelAnimationSet::GetAnimationSet("Robot"), VoxelSprite::GetVoxelSprite("Robot_idle_0"));
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
	m_animator->Play("idle");
}


//-----------------------------------------------------------------------------------------------
// Returns the velocity of the entity
//
Vector3 DynamicEntity::GetVelocity() const
{
	return m_velocity;
}


//-----------------------------------------------------------------------------------------------
// Adds the given force, to be applied during the physics step
//
void DynamicEntity::AddForce(const Vector3& force)
{
	m_force += force;
}


//-----------------------------------------------------------------------------------------------
// Adds the given impulse, to be applied during the physics step (force that ignores delta time,
// immediate change in velocity with mass consideration)
//
void DynamicEntity::AddImpulse(const Vector3& impulse)
{
	m_impulse += impulse;
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
#include "Engine/Core/LogSystem.hpp"
//-----------------------------------------------------------------------------------------------
// Performs the forward Euler physics step on the entity
//
void DynamicEntity::ApplyPhysicsStep()
{
	// Apply impulse, no delta time applied
	Vector3 deltaVelocityFromImpulse = (m_impulse * m_inverseMass);
	m_velocity += deltaVelocityFromImpulse;

	// Apply force
	if (m_affectedByGravity)
	{
		m_force += Vector3::DIRECTION_DOWN * m_mass * GRAVITY_MAGNITUDE;
	}

	Vector3 acceleration = (m_force * m_inverseMass);
	float currAcceleration = acceleration.NormalizeAndGetLength();
	currAcceleration = ClampFloat(currAcceleration, 0.f, m_maxAcceleration);
	acceleration *= currAcceleration;

	float deltaTime = Game::GetDeltaTime();
	m_velocity += (acceleration * deltaTime);

	// Clamp velocity - component wise
// 	Vector2 lateralVelocity = m_velocity.xz();
// 	float lateralSpeed = lateralVelocity.NormalizeAndGetLength();
// 	lateralSpeed = ClampFloat(lateralSpeed, 0.f, m_maxSpeed);
// 	lateralVelocity *= lateralSpeed;
// 	m_velocity = Vector3(lateralVelocity.x, m_velocity.y, lateralVelocity.y);
	m_velocity = ClampVector3(m_velocity, -m_maxSpeed, m_maxSpeed);

	// Apply velocity
	m_position += (m_velocity * deltaTime);

	// Zero out force and impulse
	m_force = Vector3::ZERO;
	m_impulse = Vector3::ZERO;
}
