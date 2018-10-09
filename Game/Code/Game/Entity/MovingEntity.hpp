/************************************************************************/
/* File: MovingEntity.hpp
/* Author: Andrew Chase
/* Date: October 9th, 2018
/* Description: Class to represent an entity that can move itself
/************************************************************************/
#pragma once
#include "Game/Entity/Entity.hpp"

class MovingEntity : public Entity
{
public:
	//-----Public Methods-----
	
	MovingEntity(const EntityDefinition* definition);
	virtual ~MovingEntity();

	virtual void Update() override;

	// Events
	virtual void	OnCollision(Entity* other) override;
	virtual void	OnDamageTaken(int damageAmount) override;
	virtual void	OnDeath() override;
	virtual void	OnSpawn() override;
	

public:
	//-----Public Methods-----

	void Move(const Vector2& direction);
	void Jump();
	void Decelerate();

	
protected:
	//-----Protected Data-----
	
// 	float	m_maxMoveAcceleration = 300.f;
// 	float	m_maxMoveSpeed = 40.f;
// 	float	m_maxMoveDeceleration = 100.f;
// 	float	m_jumpImpulse = 80.f;

// 	float	m_maxMoveAcceleration = 3000.f;
// 	float	m_maxMoveSpeed = 40.f;
// 	float	m_maxMoveDeceleration = 100.f;
// 	float	m_jumpImpulse = 80.f;

};
