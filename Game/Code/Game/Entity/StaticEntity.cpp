/************************************************************************/
/* File: StaticEntity.cpp
/* Author: Andrew Chase
/* Date: September 28th 2018
/* Description: Implementation of the static entity class
/************************************************************************/
#include "Game/Entity/StaticEntity.hpp"
#include "Engine/Math/MathUtils.hpp"


//-----------------------------------------------------------------------------------------------
// Constructor
//
StaticEntity::StaticEntity()
	: Entity(ENTITY_TYPE_STATIC)
{
	// Make it unmovable from collisions
	m_mass = REALLY_BIG_MASS;

	SetPosition(Vector3(GetRandomFloatInRange(10.f, 250.f), 4.f, GetRandomIntInRange(10.f, 250.f)));
	m_collisionDef = CollisionDefinition_t(COLLISION_SHAPE_BOX, COLLISION_RESPONSE_FULL_CORRECTION, 4.f, 4.f, 8.f);

	SetupVoxelTextures("Data/3DTextures/TestBox.qef");
}


//-----------------------------------------------------------------------------------------------
// Destructor
//
StaticEntity::~StaticEntity()
{
}


//-----------------------------------------------------------------------------------------------
// Update
//
void StaticEntity::Update()
{
	Entity::Update();
}


//-----------------------------------------------------------------------------------------------
// On collision event handler
//
void StaticEntity::OnCollision(Entity* other)
{
	Entity::OnCollision(other);
}


//-----------------------------------------------------------------------------------------------
// On damage taken event handler
//
void StaticEntity::OnDamageTaken(int damageAmount)
{
	Entity::OnDamageTaken(damageAmount);
}


//-----------------------------------------------------------------------------------------------
// On death event handler
//
void StaticEntity::OnDeath()
{
	Entity::OnDeath();
}


//-----------------------------------------------------------------------------------------------
// On spawn event handler
//
void StaticEntity::OnSpawn()
{
	Entity::OnSpawn();
}
