/************************************************************************/
/* File: MovingEntity.cpp
/* Author: Andrew Chase
/* Date: October 15th 2018
/* Description: Implementation of the MovingEntity class
/************************************************************************/
#include "Game/Framework/Game.hpp"
#include "Game/Framework/World.hpp"
#include "Game/Entity/MovingEntity.hpp"
#include "Game/Entity/Components/PhysicsComponent.hpp"
#include "Engine/Math/MathUtils.hpp"


//-----------------------------------------------------------------------------------------------
// Constructor
//
MovingEntity::MovingEntity(const EntityDefinition* definition)
	: Entity(definition)
{
}


//-----------------------------------------------------------------------------------------------
// Destructor
//
MovingEntity::~MovingEntity()
{

}


//-----------------------------------------------------------------------------------------------
// Update
//
void MovingEntity::Update()
{
	Entity::Update();
}


//-----------------------------------------------------------------------------------------------
// Collision event
//
void MovingEntity::OnCollision(Entity* other)
{
	Entity::OnCollision(other);
}


//-----------------------------------------------------------------------------------------------
// Damage event
//
void MovingEntity::OnDamageTaken(int damageAmount)
{
	Entity::OnDamageTaken(damageAmount);
}


//-----------------------------------------------------------------------------------------------
// Death event
//
void MovingEntity::OnDeath()
{
	Entity::OnDeath();
}


//-----------------------------------------------------------------------------------------------
// Spawn Event
//
void MovingEntity::OnSpawn()
{
	Entity::OnSpawn();
}


//-----------------------------------------------------------------------------------------------
// Moves the entity in the given direction, clamping to the entity's max move speed
//
void MovingEntity::Move(const Vector2& direction)
{
	float currLateralSpeed = m_physicsComponent->GetVelocity().xz().GetLength();
	float deltaTime = Game::GetDeltaTime();

	Vector2 maxLateralVelocity = (m_physicsComponent->GetVelocity().xz() + (m_definition->m_maxMoveAcceleration * deltaTime) * direction);
	float maxLateralSpeed = maxLateralVelocity.NormalizeAndGetLength();

	maxLateralSpeed = (currLateralSpeed > m_definition->m_maxMoveSpeed ? ClampFloat(maxLateralSpeed, 0.f, currLateralSpeed) : ClampFloat(maxLateralSpeed, 0.f, m_definition->m_maxMoveSpeed));
	maxLateralVelocity *= maxLateralSpeed;

	Vector3 inputVelocityResult = Vector3(maxLateralVelocity.x, m_physicsComponent->GetVelocity().y, maxLateralVelocity.y) - m_physicsComponent->GetVelocity();
	Vector3 acceleration = inputVelocityResult / deltaTime;
	Vector3 force = acceleration * m_mass;

	m_physicsComponent->AddForce(force);

	// Reorient the entity
	m_orientation = direction.GetOrientationDegrees();
}


//-----------------------------------------------------------------------------------------------
// Adds an impulse to the entity so it jumps upwards, but only if it's on the ground
//
void MovingEntity::Jump()
{
	unsigned int groundElevation = Game::GetWorld()->GetGroundElevation();

	if (m_position.y == (float)groundElevation)
	{
		m_physicsComponent->AddImpulse(Vector3::DIRECTION_UP * m_definition->m_jumpImpulse);
	}
}


//-----------------------------------------------------------------------------------------------
// Slows the entity down by applying a force against its current velocity
//
void MovingEntity::Decelerate()
{
	float deltaTime = Game::GetDeltaTime();
	float currSpeed = m_physicsComponent->GetVelocity().xz().GetLength();
	float amountCanBeDecreased = currSpeed;

	if (amountCanBeDecreased > 0.f)
	{
		Vector2 direction = -1.0f * m_physicsComponent->GetVelocity().xz().GetNormalized();

		float decelMag = amountCanBeDecreased / deltaTime;
		decelMag = ClampFloat(decelMag, 0.f, m_definition->m_maxMoveDeceleration);

		float forceMag = decelMag * m_mass;

		direction *= forceMag;
		Vector3 finalForce = Vector3(direction.x, 0.f, direction.y);

		m_physicsComponent->AddForce(finalForce);
	}
}
