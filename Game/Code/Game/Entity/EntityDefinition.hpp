/************************************************************************/
/* File: EntityDefinition.hpp
/* Author: Andrew Chase
/* Date: October 2nd, 2018
/* Description: Class to represent the data values behind an entity
/************************************************************************/
#pragma once
#include "Engine/Core/Utility/XmlUtilities.hpp"

// Convenience type to know what kind of entity we are
enum ePhysicsType
{
	ENTITY_TYPE_STATIC,
	ENTITY_TYPE_DYNAMIC,
	ENTITY_TYPE_UNASSIGNED
};

// Shape of the collision boundary
enum eCollisionShape
{
	COLLISION_SHAPE_NONE,
	COLLISION_SHAPE_DISC,
	COLLISION_SHAPE_BOX,
	NUM_COLLISION_SHAPES
};

// How the entity should react to collision corrections
enum eCollisionResponse
{
	COLLISION_RESPONSE_NO_CORRECTION,
	COLLISION_RESPONSE_SHARE_CORRECTION,
	COLLISION_RESPONSE_FULL_CORRECTION,
	NUM_COLLISION_RESPONSES
};

// Collision state for a single entity
struct CollisionDefinition_t
{
	CollisionDefinition_t()
		: m_shape(COLLISION_SHAPE_DISC), m_response(COLLISION_RESPONSE_FULL_CORRECTION), m_xExtent(4.f), m_zExtent(4.f), m_height(8.f) {}

	CollisionDefinition_t(eCollisionShape shape, eCollisionResponse type, float width, float length, float height)
		: m_shape(shape), m_response(type), m_xExtent(width), m_zExtent(length), m_height(height) {}

	eCollisionShape		m_shape;
	eCollisionResponse	m_response;
	float				m_xExtent;
	float				m_zExtent;
	float				m_height;
};


class EntityDefinition
{
	friend class Entity;

public:
	//-----Public Methods-----
	

	static void LoadDefinitions(const std::string& filename);


private:
	//-----Private Methods-----
	
	EntityDefinition(const XMLElement& entityElement);

	
private:
	//-----Private Data-----
	
	const VoxelAnimationSet*	m_animationSet = nullptr;
	CollisionDefinition_t		m_collisionDef;
	ePhysicsType				m_physicsType = ENTITY_TYPE_UNASSIGNED;

};
