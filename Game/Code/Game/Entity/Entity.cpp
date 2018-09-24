/************************************************************************/
/* File: Entity.cpp
/* Author: Andrew Chase
/* Date: September 22nd, 2017
/* Description: Implementation of the Entity class
/************************************************************************/
#include "Game/Entity/Entity.hpp"
#include "Game/Framework/Game.hpp"
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
	m_textures[0] = AssetDB::CreateOrGet3DVoxelTextureInstance("Data/3DTextures/TestCube.qef");

	// Rotate to get the other 3
	IntVector3 dimensions = m_textures[0]->GetDimensions();

	// South
	int destIndex = 0;
	m_textures[DIRECTION_SOUTH] = m_textures[0]->Copy();
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
	m_textures[DIRECTION_EAST] = m_textures[0]->Copy();
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
	m_textures[DIRECTION_WEST] = m_textures[0]->Copy();
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
	DebugRenderSystem::DrawUVSphere(m_position, 0.f, Rgba::WHITE, m_collisionRadius);
}


//-----------------------------------------------------------------------------------------------
// Callback for when collision is detected
//
void Entity::OnCollision(Entity* other)
{
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
// Returns the world position of the entity
//
Vector3 Entity::GetPosition() const
{
	return m_position;
}


//-----------------------------------------------------------------------------------------------
// Returns the 3D texture to used for rendering, based on the current 2D orientation of the entity
//
Texture3D* Entity::GetTextureForOrientation() const
{
	float cardinalAngle = GetNearestCardinalAngle(m_orientation);

	if		(cardinalAngle == 0.f)		{ return m_textures[DIRECTION_EAST]; }
	else if (cardinalAngle == 90.f)		{ return m_textures[DIRECTION_NORTH]; }
	else if (cardinalAngle == 180.f)	{ return m_textures[DIRECTION_WEST]; }
	else								{ return m_textures[DIRECTION_SOUTH]; }
}


//-----------------------------------------------------------------------------------------------
// Returns the collision radius of the entity
//
float Entity::GetCollisionRadius() const
{
	return m_collisionRadius;
}


//-----------------------------------------------------------------------------------------------
// Returns whether this entity has been marked to be deleted at the end of the frame
//
bool Entity::IsMarkedForDelete() const
{
	return m_isMarkedForDelete;
}
