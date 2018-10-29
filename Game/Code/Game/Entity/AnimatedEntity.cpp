/************************************************************************/
/* File: MovingEntity.cpp
/* Author: Andrew Chase
/* Date: October 15th 2018
/* Description: Implementation of the MovingEntity class
/************************************************************************/
#include "Game/Framework/Game.hpp"
#include "Game/Framework/World.hpp"
#include "Game/Entity/AnimatedEntity.hpp"
#include "Game/Animation/VoxelAnimator.hpp"
#include "Game/Entity/Components/PhysicsComponent.hpp"
#include "Engine/Math/MathUtils.hpp"

//-----------------------------------------------------------------------------------------------
// Constructor
//
AnimatedEntity::AnimatedEntity(const EntityDefinition* definition)
	: Entity(definition)
{
	m_animator = new VoxelAnimator(m_definition->m_animationSet, m_definition->m_defaultSprite);
	m_animator->Play("idle");
}


//-----------------------------------------------------------------------------------------------
// Destructor
//
AnimatedEntity::~AnimatedEntity()
{
	if (m_animator != nullptr)
	{
		delete m_animator;
		m_animator = nullptr;
	}
}


//-----------------------------------------------------------------------------------------------
// Update
//
void AnimatedEntity::Update()
{
	Entity::Update();
}


//-----------------------------------------------------------------------------------------------
// Returns the texture associated with the entity's current orientation and animation
//
const VoxelTexture* AnimatedEntity::GetTextureForRender() const
{
	const VoxelSprite* sprite = m_animator->GetCurrentSprite();

	if (sprite != nullptr)
	{
		return sprite->GetTextureForOrientation(m_orientation);
	}

	return nullptr;
}


//-----------------------------------------------------------------------------------------------
// Collision event
//
void AnimatedEntity::OnCollision(Entity* other)
{
	Entity::OnCollision(other);
}


//-----------------------------------------------------------------------------------------------
// Damage event
//
void AnimatedEntity::OnDamageTaken(int damageAmount)
{
	Entity::OnDamageTaken(damageAmount);
}


//-----------------------------------------------------------------------------------------------
// Death event
//
void AnimatedEntity::OnDeath()
{
	Entity::OnDeath();
}


//-----------------------------------------------------------------------------------------------
// Spawn Event
//
void AnimatedEntity::OnSpawn()
{
	Entity::OnSpawn();
}


//-----------------------------------------------------------------------------------------------
// Moves the entity in the given direction, clamping to the entity's max move speed
//
void AnimatedEntity::Move(const Vector2& direction)
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
void AnimatedEntity::Jump()
{
	unsigned int groundElevation = Game::GetWorld()->GetGroundElevation();

	if (AreMostlyEqual(m_physicsComponent->GetVelocity().y, 0.f))
	{
		m_physicsComponent->AddImpulse(Vector3::DIRECTION_UP * m_definition->m_jumpImpulse * 1.2f);
	}
}


//-----------------------------------------------------------------------------------------------
// Slows the entity down by applying a force against its current velocity
//
void AnimatedEntity::Decelerate()
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
