/************************************************************************/
/* File: Entity.hpp
/* Author: Andrew Chase
/* Date: September 22nd, 2017
/* Description: Abstract class to represent a single voxel object
/************************************************************************/
#pragma once
#include "Engine/Math/Vector3.hpp"
#include "Engine/Rendering/Resources/Texture3D.hpp"

// Direction the entity is facing, used for rendering
enum eFacingDirection
{
	DIRECTION_NORTH,
	DIRECTION_EAST,
	DIRECTION_SOUTH,
	DIRECTION_WEST, 
	NUM_DIRECTIONS
};

// Convenience type to know what kind of entity we are
enum eEntityType
{
	ENTITY_TYPE_STATIC,
	ENTITY_TYPE_DYNAMIC,
	ENTITY_TYPE_UNASSIGNED
};

class Entity
{
public:
	//-----Public Methods-----

	// Initialization
	Entity(eEntityType type);
	virtual ~Entity();

	// Core loop
	virtual void Update();

	// Collision
	virtual void OnCollision(Entity* other);

	// Mutators
	void			AddPositionOffset(const Vector3& offset);
	void			SetPosition(const Vector3& newPosition);

	// Accessors
	Vector3			GetPosition() const;
	Texture3D*		GetTextureForOrientation() const;
	float			GetCollisionRadius() const;

	// Producers
	bool			IsMarkedForDelete() const;


protected:
	//-----Protected Data-----

	Vector3			m_position = Vector3::ZERO;
	float			m_orientation = 0.f;
	bool			m_isMarkedForDelete = false;
	float			m_collisionRadius = 4.f;
	eEntityType		m_entityType = ENTITY_TYPE_UNASSIGNED;
	Texture3D*		m_textures[NUM_DIRECTIONS];

};
