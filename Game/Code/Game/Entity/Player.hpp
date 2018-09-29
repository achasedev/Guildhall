/************************************************************************/
/* File: Player.hpp
/* Author: Andrew Chase
/* Date: September 22nd, 2017
/* Description: Class to represent a player entity
/************************************************************************/
#pragma once
#include "Game/Entity/DynamicEntity.hpp"

class Vector3;
class Stopwatch;

class Player : public DynamicEntity
{
public:
	//-----Public Methods-----

	// Initialization
	Player(unsigned int playerID);
	~Player();

	// Core Loop
	void			ProcessInput();
	virtual void	Update() override;

	// Collision
	virtual void	OnCollision(Entity* other) override;
	virtual void	OnDamageTaken(int damageAmount) override;
	virtual void	OnDeath() override;
	virtual void	OnSpawn() override;

	// Behavior
	void Shoot();


private:
	//-----Private Methods-----

	void UpdateMovementParamsOnInput();
	void DebugRenderMovementParams();
	
	void ApplyInputAcceleration(const Vector2& inputDirection);
	void ApplyDeceleration();


private:
	//-----Private Data-----

	int		m_playerID = 0;
	Stopwatch* m_test;
	bool another = false;

	float	m_maxMoveAcceleration	= 300.f;
	float	m_maxMoveSpeed			= 40.f;
	float	m_maxMoveDeceleration	= 100.f;
	float	m_jumpImpulse			= 80.f;

};