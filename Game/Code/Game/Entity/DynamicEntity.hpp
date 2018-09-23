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
	virtual void Update() override;

	// Physics
	void AddForce(const Vector3& force);
	void ApplyPhysicsStep();

	// Collision
	virtual void	OnCollision(Entity* other) override;
	void			AddCollisionCorrection(const Vector3& correction);
	
	// Accessors
	float GetMass() const;
	float GetInverseMass() const;

protected:
	//-----Protected Data-----

	Vector3 m_velocity			= Vector3::ZERO;
	Vector3 m_acceleration		= Vector3::ZERO;
	Vector3 m_force				= Vector3::ZERO;
	float	m_mass				= 1.f;
	float	m_inverseMass		= 1.f;
	bool	m_affectedByGravity	= false;

};
