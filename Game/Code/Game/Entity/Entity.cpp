/************************************************************************/
/* File: Entity.cpp
/* Author: Andrew Chase
/* Date: September 22nd, 2017
/* Description: Implementation of the Entity class
/************************************************************************/
#include "Game/Entity/Entity.hpp"
#include "Game/Framework/Game.hpp"
#include "Game/Framework/GameCommon.hpp"
#include "Engine/Core/Rgba.hpp"
#include "Engine/Assets/AssetDB.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Rendering/DebugRendering/DebugRenderSystem.hpp"
#include <stdlib.h>


//-----------------------------------------------------------------------------------------------
// Constructor
//
Entity::Entity(eEntityType type)
	: m_entityType(type)
{
}


//-----------------------------------------------------------------------------------------------
// Destructor
//
Entity::~Entity()
{
	for (int i = 0; i < NUM_DIRECTIONS; ++i)
	{
		if (m_textures[i] != nullptr)
		{
			delete m_textures[i];
			m_textures[i] = nullptr;
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Update
//
void Entity::Update()
{
// 	DebugRenderOptions options;
// 	options.m_startColor = Rgba::WHITE;
// 	options.m_endColor = Rgba::WHITE;
// 	options.m_lifetime = 0.f;
// 	options.m_renderMode = DEBUG_RENDER_USE_DEPTH;
// 	options.m_isWireFrame = true;
// 
// 	if (m_collisionDef.m_type == COLLISION_TYPE_DISC)
// 	{
// 		DebugRenderSystem::DrawUVSphere(m_position, options, m_collisionDef.m_width);
// 	}
// 	else
// 	{
// 		DebugRenderSystem::DrawCube(m_position, options, Vector3(m_collisionDef.m_width, m_collisionDef.m_height, m_collisionDef.m_length));
// 	}
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
// Subtracts the amount to the entity's health
//
void Entity::TakeDamage(int damageAmount)
{
	m_health -= damageAmount;

	OnDamageTaken(damageAmount);

	if (m_health <= 0)
	{
		OnDeath();
	}
}


//-----------------------------------------------------------------------------------------------
// Called when the entity takes damage, for custom behavior
//
void Entity::OnDamageTaken(int damageAmount)
{
	UNUSED(damageAmount);
}


//-----------------------------------------------------------------------------------------------
// Called when the entity's health is <= 0
//
void Entity::OnDeath()
{
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
// Gets the voxel texture given by filename and sets it up for use with this entity
//
void Entity::SetupVoxelTextures(const char* filename)
{
	m_textures[0] = AssetDB::CreateOrGet3DVoxelTextureInstance(filename);

	// Rotate to get the other 3
	IntVector3 dimensions = m_textures[0]->GetDimensions();

	// South
	int destIndex = 0;
	m_textures[DIRECTION_SOUTH] = m_textures[0]->Clone();
	for (int y = 0; y < dimensions.y; ++y)
	{
		for (int z = dimensions.z - 1; z >= 0; --z)
		{
			for (int x = dimensions.x - 1; x >= 0; --x)
			{
				int sourceIndex = y * (dimensions.x * dimensions.z) + z * dimensions.x + x;
				m_textures[DIRECTION_SOUTH]->SetColorAtIndex(destIndex, m_textures[0]->GetColorAtIndex(sourceIndex));
				destIndex++;
			}
		}
	}

	// East
	m_textures[DIRECTION_EAST] = m_textures[0]->Clone();
	destIndex = 0;
	for (int y = 0; y < dimensions.y; ++y)
	{
		for (int x = dimensions.x - 1; x >= 0; --x)
		{
			for (int z = 0; z < dimensions.z; ++z)
			{
				int sourceIndex = y * (dimensions.x * dimensions.z) + z * dimensions.x + x;
				m_textures[DIRECTION_EAST]->SetColorAtIndex(destIndex, m_textures[0]->GetColorAtIndex(sourceIndex));

				destIndex++;
			}
		}
	}

	// West
	m_textures[DIRECTION_WEST] = m_textures[0]->Clone();
	destIndex = 0;
	for (int y = 0; y < dimensions.y; ++y)
	{
		for (int x = 0; x < dimensions.x; ++x)
		{
			for (int z = dimensions.z - 1; z >= 0; --z)
			{
				int sourceIndex = y * (dimensions.x * dimensions.z) + z * dimensions.x + x;
				m_textures[DIRECTION_WEST]->SetColorAtIndex(destIndex, m_textures[0]->GetColorAtIndex(sourceIndex));

				destIndex++;
			}
		}
	}

	m_orientation = 0;
}


//-----------------------------------------------------------------------------------------------
// Returns the world position of the entity
//
Vector3 Entity::GetEntityPosition() const
{
	return m_position;
}


//-----------------------------------------------------------------------------------------------
// Returns the 3D texture to used for rendering, based on the current 2D orientation of the entity
//
VoxelTexture* Entity::GetTextureForOrientation() const
{
	float cardinalAngle = GetNearestCardinalAngle(m_orientation);

	if		(cardinalAngle == 0.f)		{ return m_textures[DIRECTION_EAST]; }
	else if (cardinalAngle == 90.f)		{ return m_textures[DIRECTION_NORTH]; }
	else if (cardinalAngle == 180.f)	{ return m_textures[DIRECTION_WEST]; }
	else								{ return m_textures[DIRECTION_SOUTH]; }
}


//-----------------------------------------------------------------------------------------------
// Returns the collision definition of this entity
//
CollisionDefinition_t Entity::GetCollisionDefinition() const
{
	return m_collisionDef;
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
	IntVector3 dimensions = m_textures[0]->GetDimensions();
	IntVector3 halfDimensions = dimensions / 2;

	IntVector3 entityPositionCoords = GetEntityCoordinatePosition();

	IntVector3 bottomLeft = entityPositionCoords - IntVector3(halfDimensions.x, 0, halfDimensions.z);

	Vector3 position = Vector3(bottomLeft + localCoords);
	position += Vector3(0.5f, 0.f, 0.5f);

	return position;
}


//-----------------------------------------------------------------------------------------------
// Returns the position of the entity's voxel at the given index
//
Vector3 Entity::GetPositionForLocalIndex(unsigned int index) const
{
	IntVector3 dimensions = m_textures[0]->GetDimensions();

	int y = index / (dimensions.x * dimensions.z);
	int leftOver = index % (dimensions.x * dimensions.z);

	int z = leftOver / (dimensions.x);
	int x = leftOver % (dimensions.x);

	return GetPositionForLocalCoords(IntVector3(x, y, z));
}


//-----------------------------------------------------------------------------------------------
// Returns the coordinate position that this entity occupies
//
IntVector3 Entity::GetEntityCoordinatePosition() const
{
	return IntVector3(m_position);
}
