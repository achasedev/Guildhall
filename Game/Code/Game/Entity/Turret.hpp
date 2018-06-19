/************************************************************************/
/* File: Turret.hpp
/* Author: Andrew Chase
/* Date: June 18th, 2018
/* Description: Class to represent a turret on a tank, with a cannon
/************************************************************************/
#pragma once
#include "Engine/Core/GameObject.hpp"

class Cannon;

class Turret : public GameObject
{
public:
	//-----Public Methods-----

	Turret(Transform& parent);
	~Turret();

	virtual void Update(float deltaTime) override;

	// Accessors
	Cannon* GetCannon() const;

	// Mutators
	void TurnTowardsTarget(const Vector3& target);


private:
	//-----Private Data-----

	Cannon* m_cannon;

	static const float TURRET_ROTATION_SPEED;

};
