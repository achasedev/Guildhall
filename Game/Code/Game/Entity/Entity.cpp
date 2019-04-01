/************************************************************************/
/* File: Entity.cpp
/* Author: Andrew Chase
/* Date: April 1st 2019
/* Description: Implementation of the Entity class
/************************************************************************/
#include "Game/Entity/Entity.hpp"
#include "Game/Framework/Game.hpp"
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
// Returns the entity's physics bounds in world coordinates, based on its position
//
AABB3 Entity::GetWorldPhysicsBounds() const
{
	return m_localPhysicsBounds.GetTranslated(m_position);
}


//-----------------------------------------------------------------------------------------------
// Returns whether this entity should be deleted at the end of this frame
//
bool Entity::IsMarkedForDelete() const
{
	return m_isMarkedForDelete;
}
