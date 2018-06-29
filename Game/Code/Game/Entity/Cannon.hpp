/************************************************************************/
/* File: Cannon.hpp
/* Author: Andrew Chase
/* Date: June 18th, 2018
/* Description: Class to represent a cannon/gun on a turret of a tank
/************************************************************************/
#pragma once
#include "Engine/Core/GameObject.hpp"
#include "Engine/Math/FloatRange.hpp"

class Cannon : public GameObject
{
public:
	//-----Public Methods-----

	Cannon(Transform& parent);
	~Cannon();

	virtual void Update(float deltaTime) override;

	// Accessors
	Transform GetFireTransform();

	// Mutators
	void ElevateTowardsTarget(const Vector3& target);


private:
	//-----Private Data-----

	Transform m_muzzleTransform; // Spawn transform for projectiles
	const FloatRange m_angleLimits;

	static const float CANNON_ROTATION_SPEED;

};
