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

	virtual void	Update();
	void			ApplyPhysicsStep();
	virtual void	Render() const;

	// Mutators
	inline void		AddForce(const Vector3& force);
	inline void		AddImpulse(const Vector3& impulse);
	inline void		AddVelocity(const Vector3& velocity);
	inline void		AddPositionOffset(const Vector3& offsetTranslation);

	// Accessors
	AABB3			GetWorldPhysicsBounds() const;
	inline bool		IsMarkedForDelete() const;
	inline Vector3	GetVelocity() const;


private:
	//-----Statics-----

	static constexpr float ENTITY_DEFAULT_PHYSICS_LENGTH_X = 0.9f;
	static constexpr float ENTITY_DEFAULT_PHYSICS_WIDTH_Y = 0.9f;
	static constexpr float ENTITY_DEFAULT_PHYSICS_HEIGHT_Z = 1.8f;

	static const Vector3 ENTITY_DEFAULT_LOCAL_PHYSICS_BACK_LEFT_BOTTOM;
	static const Vector3 ENTITY_DEFAULT_LOCAL_PHYSICS_FRONT_RIGHT_TOP;

	static constexpr float GRAVITY_ACCELERATION = 9.8f;


private:
	//-----Private Member Data-----

	float			m_ageSeconds = 0.f;
	bool			m_isMarkedForDelete = false;
	Vector3			m_position = Vector3(10.f, 10.f, 50.f);
	float			m_xyOrientationDegrees = 0.f;

	// Physics
	AABB3			m_localPhysicsBounds = AABB3(ENTITY_DEFAULT_LOCAL_PHYSICS_BACK_LEFT_BOTTOM, ENTITY_DEFAULT_LOCAL_PHYSICS_FRONT_RIGHT_TOP);
	ePhysicsMode	m_physicsMode = PHYSICS_MODE_WALKING;

	float m_mass			= 1.0f;
	Vector3 m_velocity		= Vector3::ZERO; 
	Vector3 m_acceleration	= Vector3::ZERO;
	Vector3 m_force			= Vector3::ZERO;
	Vector3 m_impulse		= Vector3::ZERO;
	
};


//-----------------------------------------------------------------------------------------------
// Adds the given force to the net force to be applied during the physics step
//
inline void Entity::AddForce(const Vector3& force)
{
	m_force += force;
}


//-----------------------------------------------------------------------------------------------
// Adds the given impulse to the net impulse to be applied during the physics step
//
inline void Entity::AddImpulse(const Vector3& impulse)
{
	m_impulse += impulse;
}


//-----------------------------------------------------------------------------------------------
// Adds the given velocity to the existing velocity
//
inline void	Entity::AddVelocity(const Vector3& velocity)
{
	m_velocity += velocity;
}


//-----------------------------------------------------------------------------------------------
// Translates the entity by offsetTranslation
//
inline void	Entity::AddPositionOffset(const Vector3& offsetTranslation)
{
	m_position += offsetTranslation;
}


//-----------------------------------------------------------------------------------------------
// Returns whether this entity should be deleted at the end of this frame
//
inline bool Entity::IsMarkedForDelete() const
{
	return m_isMarkedForDelete;
}


//-----------------------------------------------------------------------------------------------
// Returns the velocity of the entity
//
Vector3 Entity::GetVelocity() const
{
	return m_velocity;
}
