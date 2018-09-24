/************************************************************************/
/* File: DynamicEntity.hpp
/* Author: Andrew Chase
/* Date: September 22nd, 2017
/* Description: Class to represent an Entity affected by physics
/************************************************************************/
#pragma once
#include "Engine/Math/Vector3.hpp"
#include "Game/Entity/Entity.hpp"

class DynamicEntity : public Entity
{
public:
	//-----Public Methods-----

	// Initialization
	DynamicEntity();
	~DynamicEntity();

	// Core
	virtual void	Update() override;

	// Physics		
	void			AddForce(const Vector3& force);
	void			AddVelocity(const Vector3& velocity);

	void			SetForce(const Vector3& force);
	void			SetVelocity(const Vector3& velocity);

	void			ApplyPhysicsStep();

	// Collision	
	virtual void	OnCollision(Entity* other) override;
	void			AddCollisionCorrection(const Vector3& correction);

	// Accessors	
	float			GetMass() const;
	float			GetInverseMass() const;


protected:
	//-----Protected Data-----

	// State
	Vector3 m_velocity									= Vector3::ZERO;
	Vector3 m_force										= Vector3::ZERO;

	// Data set members
	float	m_mass										= DEFAULT_MASS;					// Mass of the Entity
	float	m_inverseMass								= 1.f / DEFAULT_MASS;			// Cache off inverse for efficiency
	float	m_maxSpeed									= DEFAULT_MAX_SPEED;			// Max speed this entity can move
	float	m_maxAcceleration							= DEFAULT_MAX_ACCELERATION;		// Max change in velocity per second
	bool	m_affectedByGravity							= false;

	// Defaults
	static constexpr float DEFAULT_MAX_ACCELERATION		= 100000.f;
	static constexpr float DEFAULT_MAX_SPEED			= 100.f;
	static constexpr float DEFAULT_MASS					= 1.0f;

};
