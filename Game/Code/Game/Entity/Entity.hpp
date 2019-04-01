/************************************************************************/
/* File: Entity.hpp
/* Author: Andrew Chase
/* Date: April 1st 2019
/* Description: Class to represent an object/actor in the scene
/************************************************************************/
#pragma once
#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/Vector3.hpp"

enum ePhysicsMode
{
	PHYSICS_MODE_WALKING,
	PHYSICS_MODE_FLYING,
	PHYSICS_MODE_NO_CLIP
};

class Entity
{
public:
	//-----Public Methods-----

	virtual void Update();
	virtual void Render() const;

	// Accessors
	AABB3	GetWorldPhysicsBounds() const;
	bool	IsMarkedForDelete() const;


private:
	//-----Statics-----

	static constexpr float ENTITY_DEFAULT_PHYSICS_LENGTH_X = 0.9f;
	static constexpr float ENTITY_DEFAULT_PHYSICS_WIDTH_Y = 0.9f;
	static constexpr float ENTITY_DEFAULT_PHYSICS_HEIGHT_Z = 1.8f;

	static const Vector3 ENTITY_DEFAULT_LOCAL_PHYSICS_BACK_LEFT_BOTTOM;
	static const Vector3 ENTITY_DEFAULT_LOCAL_PHYSICS_FRONT_RIGHT_TOP;


private:
	//-----Private Member Data-----

	float			m_ageSeconds = 0.f;
	bool			m_isMarkedForDelete = false;
	Vector3			m_position = Vector3(10.f, 10.f, 50.f);
	float			m_xyOrientationDegrees = 0.f;

	AABB3			m_localPhysicsBounds = AABB3(ENTITY_DEFAULT_LOCAL_PHYSICS_BACK_LEFT_BOTTOM, ENTITY_DEFAULT_LOCAL_PHYSICS_FRONT_RIGHT_TOP);
	ePhysicsMode	m_physicsMode = PHYSICS_MODE_WALKING;
};
