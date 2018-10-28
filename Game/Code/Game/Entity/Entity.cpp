/************************************************************************/
/* File: Entity.cpp
/* Author: Andrew Chase
/* Date: September 22nd, 2017
/* Description: Implementation of the Entity class
/************************************************************************/
#include "Game/Entity/Entity.hpp"
#include "Game/Framework/Game.hpp"
#include "Game/Framework/GameCommon.hpp"
#include "Game/Animation/VoxelSprite.hpp"
#include "Game/Entity/Components/PhysicsComponent.hpp"
#include "Engine/Core/Rgba.hpp"
#include "Engine/Assets/AssetDB.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Rendering/DebugRendering/DebugRenderSystem.hpp"
#include <stdlib.h>


//-----------------------------------------------------------------------------------------------
// Constructor - takes the definition for its constant data characteristics
//
Entity::Entity(const EntityDefinition* definition)
	: m_definition(definition)
{
	if (m_definition->m_physicsType == PHYSICS_TYPE_DYNAMIC)
	{
		m_physicsComponent = new PhysicsComponent(this);
	}

	m_position = Vector3(GetRandomFloatInRange(10.f, 50.f), 4.f, GetRandomFloatInRange(10.f, 200.f));

	// Only create a default texture if the definition has one specified
	if (definition->m_defaultSprite != nullptr)
	{
		m_defaultTexture = definition->m_defaultSprite->GetTextureForOrientation(90.f)->Clone();
	}
}


//-----------------------------------------------------------------------------------------------
// Destructor
//
Entity::~Entity()
{
	if (m_physicsComponent != nullptr)
	{
		delete m_physicsComponent;
		m_physicsComponent = nullptr;
	}
	
	if (m_defaultTexture != nullptr)
	{
		delete m_defaultTexture;
		m_defaultTexture = nullptr;
	}
}


//-----------------------------------------------------------------------------------------------
// Update
//
void Entity::Update()
{
}


//-----------------------------------------------------------------------------------------------
// Callback for when collision is detected
//
void Entity::OnCollision(Entity* other)
{
	UNUSED(other);
}


//-----------------------------------------------------------------------------------------------
// Adds an offset (translates) to the player
//
void Entity::AddPositionOffset(const Vector3& offset)
{
	m_position += offset;
}


//-----------------------------------------------------------------------------------------------
// Hard sets the position of the entity
//
void Entity::SetPosition(const Vector3& newPosition)
{
	m_position = newPosition;
}


//-----------------------------------------------------------------------------------------------
// Sets the orientation of the entity to the provided value
//
void Entity::SetOrientation(float orientation)
{
	m_orientation = orientation;
}


//-----------------------------------------------------------------------------------------------
// Sets the team of the entity to the one specified
//
void Entity::SetTeam(eEntityTeam team)
{
	m_entityTeam = team;
}


//-----------------------------------------------------------------------------------------------
// Subtracts the amount to the entity's health
//
void Entity::TakeDamage(int damageAmount)
{
	m_health -= damageAmount;

	OnDamageTaken(damageAmount);
}


//-----------------------------------------------------------------------------------------------
// Called when the entity takes damage, for custom behavior
//
void Entity::OnDamageTaken(int damageAmount)
{
	UNUSED(damageAmount);

	if (m_health <= 0)
	{
		OnDeath();
	}
}


//-----------------------------------------------------------------------------------------------
// Called when the entity's health is <= 0
//
void Entity::OnDeath()
{
	m_isMarkedForDelete = true;
}


//-----------------------------------------------------------------------------------------------
// Called when the entity is spawned
//
void Entity::OnSpawn()
{
}


//-----------------------------------------------------------------------------------------------
// Adds the correction to the entity's position to fix a collision
//
void Entity::AddCollisionCorrection(const Vector3& correction)
{
	m_position += correction;
}


//-----------------------------------------------------------------------------------------------
// Returns the world position of the entity
//
Vector3 Entity::GetPosition() const
{
	return m_position;
}


//-----------------------------------------------------------------------------------------------
// Returns the orientation of the entity
//
float Entity::GetOrientation() const
{
	return m_orientation;
}


//-----------------------------------------------------------------------------------------------
// Returns the team index of the entity
//
eEntityTeam Entity::GetTeam() const
{
	return m_entityTeam;
}


//-----------------------------------------------------------------------------------------------
// Returns the 3D texture to used for rendering, based on the current 2D orientation of the entity
//
const VoxelTexture* Entity::GetTextureForRender() const
{
	return m_defaultTexture;
}


//-----------------------------------------------------------------------------------------------
// Returns the collision definition of this entity
//
CollisionDefinition_t Entity::GetCollisionDefinition() const
{
	return m_definition->m_collisionDef;
}


//-----------------------------------------------------------------------------------------------
// Returns the physics type of the entity
//
ePhysicsType Entity::GetPhysicsType() const
{
	return m_definition->m_physicsType;
}


//-----------------------------------------------------------------------------------------------
// Returns the definition used to describe this entity's data characteristics
//
const EntityDefinition* Entity::GetEntityDefinition() const
{
	return m_definition;
}


//-----------------------------------------------------------------------------------------------
// Returns the physics component of this entity
//
PhysicsComponent* Entity::GetPhysicsComponent() const
{
	return m_physicsComponent;
}


//-----------------------------------------------------------------------------------------------
// Returns the dimensions of the entity
//
IntVector3 Entity::GetDimensions() const
{
	const VoxelTexture* texture = GetTextureForRender();
	return texture->GetDimensions();
}


//-----------------------------------------------------------------------------------------------
// Returns the mass of the entity
//
float Entity::GetMass() const
{
	return m_mass;
}


//-----------------------------------------------------------------------------------------------
// Returns one over the mass of the entity
//
float Entity::GetInverseMass() const
{
	return m_inverseMass;
}


//-----------------------------------------------------------------------------------------------
// Returns whether this entity is a player entitys
//
bool Entity::IsPlayer() const
{
	return m_isPlayer;
}


//-----------------------------------------------------------------------------------------------
// Returns whether this entity has been marked to be deleted at the end of the frame
//
bool Entity::IsMarkedForDelete() const
{
	return m_isMarkedForDelete;
}


//-----------------------------------------------------------------------------------------------
// Returns the position of the entity's voxel at the given local coords
//
Vector3 Entity::GetPositionForLocalCoords(const IntVector3& localCoords) const
{
	IntVector3 bottomLeft = GetCoordinatePosition();

	Vector3 position = Vector3(bottomLeft + localCoords);
	position += Vector3(0.5f, 0.f, 0.5f);

	return position;
}


//-----------------------------------------------------------------------------------------------
// Returns the position of the entity's voxel at the given index
//
Vector3 Entity::GetPositionForLocalIndex(unsigned int index) const
{
	IntVector3 dimensions = GetDimensions();

	int y = index / (dimensions.x * dimensions.z);
	int leftOver = index % (dimensions.x * dimensions.z);

	int z = leftOver / (dimensions.x);
	int x = leftOver % (dimensions.x);

	return GetPositionForLocalCoords(IntVector3(x, y, z));
}


//-----------------------------------------------------------------------------------------------
// Returns the coordinate position that this entity occupies
//
IntVector3 Entity::GetCoordinatePosition() const
{
	return IntVector3(RoundToNearestInt(m_position.x), RoundToNearestInt(m_position.y), RoundToNearestInt(m_position.z));
}
