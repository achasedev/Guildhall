/************************************************************************/
/* File: PhysicsComponent.hpp
/* Author: Andrew Chase
/* Date: October 8th, 2018
/* Description: Class to update the entity using a forward Euler physics model
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
	void			ZeroXVelocity();
	void			ZeroYVelocity();
	void			ZeroZVelocity();

	void			SetGravity(bool hasGravity);
	void			StopAllMovement();

	void			ApplyPhysicsStep();

	// Accessors	
	Vector3			GetVelocity() const;



protected:
	//-----Protected Data-----

	Entity* m_owningEntity = nullptr;

	// State
	Vector3 m_force = Vector3::ZERO;
	Vector3 m_impulse = Vector3::ZERO;
	Vector3 m_velocity = Vector3::ZERO;

	// Data set members
	float	m_maxSpeed = DEFAULT_MAX_SPEED;						// Max speed this entity can move
	float	m_maxAcceleration = DEFAULT_MAX_ACCELERATION;		// Max change in velocity per second
	bool	m_affectedByGravity = false;

	// Defaults
	static constexpr float DEFAULT_MAX_ACCELERATION = 1000000.f;
	static constexpr float DEFAULT_MAX_SPEED = 1000.f;

};
