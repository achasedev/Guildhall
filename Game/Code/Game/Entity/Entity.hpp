/************************************************************************/
/* File: Entity.hpp
/* Author: Andrew Chase
/* Date: April 1st 2019
/* Description: Class to represent an object/actor in the scene
/************************************************************************/
#pragma once
#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/Vector3.hpp"

enum ePhysicsMode
{
	PHYSICS_MODE_WALKING,
	PHYSICS_MODE_FLYING,
	PHYSICS_MODE_NO_CLIP,
	NUM_PHYSICS_MODES
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
	inline void		SetIsOnGround(bool isOnGround);
	inline void		SetPhysicsMode(ePhysicsMode newMode);

	void			MoveSelfHorizontal(const Vector2& directionToMove);
	void			MoveSelfVertical(const float directionToMove);

	void			Jump();
	void			ApplyHorizontalFrictionOrAirDrag();
	void			ApplyVerticalAirDrag();

	// Accessors
	AABB3			GetWorldPhysicsBounds() const;
	inline bool		IsMarkedForDelete() const;
	inline Vector3	GetPosition() const;
	inline float	GetXYOrientationDegrees() const;
	inline Vector3	GetVelocity() const;
	inline bool		IsOnGround() const;
	inline Vector3	GetEyeWorldPosition() const;
	inline Vector3	GetForwardVector() const;
	inline Vector3	GetCenterWorldPosition() const;
	inline ePhysicsMode GetPhysicsMode() const;


protected:
	//-----Statics-----

	static constexpr float ENTITY_DEFAULT_PHYSICS_LENGTH_X = 0.6f;
	static constexpr float ENTITY_DEFAULT_PHYSICS_WIDTH_Y = 0.6f;
	static constexpr float ENTITY_DEFAULT_PHYSICS_HEIGHT_Z = 1.8f;
	static constexpr float ENTITY_DEFAULT_EYE_HEIGHT = 1.65f;

	static const Vector3 ENTITY_DEFAULT_LOCAL_PHYSICS_BACK_LEFT_BOTTOM;
	static const Vector3 ENTITY_DEFAULT_LOCAL_PHYSICS_FRONT_RIGHT_TOP;

	static constexpr float ENTITY_GRAVITY_ACCELERATION = 15.f;
	static constexpr float ENTITY_GROUND_FRICTION_DECELERATION = 16.0f;
	static constexpr float ENTITY_AIR_DRAG_DECELERATION = 12.f;
	static constexpr float ENTITY_DEFAULT_MAX_Z_MOVE_SPEED = 10.f;
	static constexpr float ENTITY_DEFAULT_MAX_XY_WALK_SPEED = 5.f;
	static constexpr float ENTITY_DEFAULT_MAX_XY_FLY_SPEED = 10.f;
	static constexpr float ENTITY_DEFAULT_MOVE_ACCELERATION = 40.f;
	static constexpr float ENTITY_DEFAULT_JUMP_HEIGHT = 1.4f;


protected:
	//-----Protected Member Data-----

	float			m_ageSeconds = 0.f;
	bool			m_isMarkedForDelete = false;
	Vector3			m_position = Vector3(10.f, 10.f, 50.f);
	float			m_xyOrientationDegrees = 0.f;

	// Physics
	AABB3			m_localPhysicsBounds = AABB3(ENTITY_DEFAULT_LOCAL_PHYSICS_BACK_LEFT_BOTTOM, ENTITY_DEFAULT_LOCAL_PHYSICS_FRONT_RIGHT_TOP);
	ePhysicsMode	m_physicsMode = PHYSICS_MODE_WALKING;

	bool	m_isOnGround		= false;
	float	m_mass				= 1.0f;
	Vector3 m_velocity			= Vector3::ZERO; 
	Vector3 m_force				= Vector3::ZERO;
	Vector3 m_impulse			= Vector3::ZERO;
	float	m_maxXYWalkSpeed	= ENTITY_DEFAULT_MAX_XY_WALK_SPEED;
	float	m_maxXYFlySpeed		= ENTITY_DEFAULT_MAX_XY_FLY_SPEED;
	float	m_maxZMoveSpeed		= ENTITY_DEFAULT_MAX_Z_MOVE_SPEED;
	float	m_moveAcceleration	= ENTITY_DEFAULT_MOVE_ACCELERATION;
	float	m_jumpHeight		= ENTITY_DEFAULT_JUMP_HEIGHT;
	Vector3	m_eyeOffsetFromPosition = Vector3(0.f, 0.f, ENTITY_DEFAULT_EYE_HEIGHT);

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
// Returns the world position of the entity
//
inline Vector3 Entity::GetPosition() const
{
	return m_position;
}


//-----------------------------------------------------------------------------------------------
// Returns the XY orientation of the entity
//
inline float Entity::GetXYOrientationDegrees() const
{
	return m_xyOrientationDegrees;
}


//-----------------------------------------------------------------------------------------------
// Returns the velocity of the entity
//
Vector3 Entity::GetVelocity() const
{
	return m_velocity;
}


//-----------------------------------------------------------------------------------------------
// Returns whether this entity is flagged as standing on the ground
//
inline bool	Entity::IsOnGround() const
{
	return m_isOnGround;
}


//-----------------------------------------------------------------------------------------------
// Sets the flag indicating whether this entity is resting on the ground this frame
//
inline void	Entity::SetIsOnGround(bool isOnGround)
{
	m_isOnGround = isOnGround;
}


//-----------------------------------------------------------------------------------------------
// Sets the physics mode of the entity
//
inline void	Entity::SetPhysicsMode(ePhysicsMode newMode)
{
	m_physicsMode = newMode;
}


//-----------------------------------------------------------------------------------------------
// Returns where the entity's eyes are located in world space
//
inline Vector3 Entity::GetEyeWorldPosition() const
{
	return m_position + m_eyeOffsetFromPosition;
}


//-----------------------------------------------------------------------------------------------
// Returns the entity's forward vector in the xy plane
//
inline Vector3 Entity::GetForwardVector() const
{
	Vector2 xyForward = Vector2::MakeDirectionAtDegrees(m_xyOrientationDegrees);
	return Vector3(xyForward.x, xyForward.y, 0.f);
}


//-----------------------------------------------------------------------------------------------
// Returns the position for the center (x,y and z) of the entity's collision box
//
inline Vector3 Entity::GetCenterWorldPosition() const
{
	return m_localPhysicsBounds.GetCenter() + m_position;
}


//-----------------------------------------------------------------------------------------------
// Returns the current physics mode of the entity
//
inline ePhysicsMode Entity::GetPhysicsMode() const
{
	return m_physicsMode;
}
