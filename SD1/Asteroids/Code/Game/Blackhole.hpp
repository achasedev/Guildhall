/************************************************************************/
/* Project: SD1 C27 Asteroids Gold
/* File: Blackhole.hpp
/* Author: Andrew Chase
/* Date: September 19th, 2017
/* Bugs: None
/* Description: Class to represent a black hole that pulls game Entities
/************************************************************************/
#pragma once
#include "Game/Entity.hpp"


class Blackhole : public Entity
{

public: 
	//-----Public Methods-----

	Blackhole();

	void Update(float deltaTime);
	void Render() const;

	float GetGravityRadius() const;

public:

	//-----Public Data-----

	static constexpr float GRAVITY_SCALAR = 500.f;	// Force strength of the black hole


private:

	//-----Private Data-----

	float m_gravityRadius;							// Range at which the black hole can pull Entities
};
