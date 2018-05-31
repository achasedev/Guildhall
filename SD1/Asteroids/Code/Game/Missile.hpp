/************************************************************************/
/* Project: MP1 C27 A1 - Vector2, Basics, MathUtils
/* File: Missile.hpp
/* Author: Andrew Chase
/* Date: September 21st, 2017
/* Bugs: None
/* Description: Implementation of the Missile class
/************************************************************************/
#pragma once
#include "Game/Entity.hpp"
#include "Game/Asteroid.hpp"
#include "Engine/Core/Rgba.hpp"


class Missile : public Entity
{

public:

	//-----Public Methods-----

	Missile(Vector2 nosePosition, Vector2 fireDirection, float shipOrientation, Asteroid* target);

	void Update(float deltaTime);
	void Render() const;

	void SetTarget(Asteroid* newTarget);

public:

private:

	//-----Private Methods-----

	void TurnTowardsTarget(float deltaTime);

private:

	//-----Private Data-----

	bool m_hasTarget = false;
	Asteroid* m_target = nullptr;

	static constexpr float BASE_MISSILE_SPEED = 300.f;
	static constexpr float MISSILE_TURN_SPEED = 360.f;
	static constexpr float MISSILE_ACCELERATION = 500.f;
};