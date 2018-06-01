/************************************************************************/
/* File: Zombie.hpp
/* Author: Andrew Chase
/* Date: May 9th, 2018
/* Description: Class to represet a Miku Zombie
/************************************************************************/
#pragma once
#include "Engine/Core/GameObject.hpp"

class Zombie : public GameObject
{
public:
	//-----Public Methods-----

	Zombie();
	~Zombie();

	virtual void Update(float deltaTime) override;


private:
	//-----Private Data-----

	int m_health;
	Vector3 m_targetPosition;

	// Statics
	static float s_turnSpeed;
	static float s_walkSpeed;

	static constexpr float ZOMBIE_INITIAL_WALK_SPEED = 0.1f;
	static constexpr float ZOMBIE_INITIAL_TURN_SPEED = 90.f;
};