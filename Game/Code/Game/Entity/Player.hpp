/************************************************************************/
/* File: Player.hpp
/* Author: Andrew Chase
/* Date: September 22nd, 2017
/* Description: Class to represent a player entity
/************************************************************************/
#pragma once
#include "Game/Entity/MovingEntity.hpp"

#define INVALID_PLAYER_ID (4)

class Vector3;

class Player : public MovingEntity
{
public:
	//-----Public Methods-----

	// Initialization
	Player(unsigned int playerID);
	~Player();

	// Core Loop
	void			ProcessInput();
	virtual void	Update() override;

	// Events
	virtual void	OnCollision(Entity* other) override;
	virtual void	OnDamageTaken(int damageAmount) override;
	virtual void	OnDeath() override;
	virtual void	OnSpawn() override;

	// Behavior
	void Shoot();


private:
	//-----Private Methods-----


private:
	//-----Private Data-----

	int		m_playerID = INVALID_PLAYER_ID;

};