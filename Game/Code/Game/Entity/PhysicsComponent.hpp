/************************************************************************/
/* File: DynamicEntity.hpp
/* Author: Andrew Chase
/* Date: September 22nd, 2017
/* Description: Class to represent an Entity affected by physics
/************************************************************************/
#pragma once
#include "Game/Entity/Entity.hpp"
#include "Engine/Math/IntVector3.hpp"

class PhysicsComponent
{
public:
	//-----Public Methods-----

	// Initialization
	PhysicsComponent(Entity* entity);
	~PhysicsComponent();

	// Physics		
	void			AddForce(const Vector3& force);
	void			AddImpulse(const Vector3& impulse);
	void			AddVelocity(const Vector3& velocity);

	void			SetForce(const Vector3& force);
	void			SetVelocity(const Vector3& velocity);

	void			ApplyPhysicsStep();

	// Accessors	
	Vector3			GetVelocity() const;


protected:
	//-----Protected Methods-----


protected:
	//-----Protected Data-----

	Entity* m_owningEntity = nullptr;

	// State
	Vector3 m_force = Vector3::ZERO;
	Vector3 m_impulse = Vector3::ZERO;
	Vector3 m_velocity = Vector3::ZERO;

	// Data set members
	float	m_maxSpeed = DEFAULT_MAX_SPEED;			// Max speed this entity can move
	float	m_maxAcceleration = DEFAULT_MAX_ACCELERATION;		// Max change in velocity per second
	bool	m_affectedByGravity = false;

	// Defaults
	static constexpr float DEFAULT_MAX_ACCELERATION = 1000000.f;
	static constexpr float DEFAULT_MAX_SPEED = 1000.f;

};
