/************************************************************************/
/* File: Player.hpp
/* Author: Andrew Chase
/* Date: September 22nd, 2017
/* Description: Class to represent a player entity
/************************************************************************/
#pragma once
#include "Game/Entity/AnimatedEntity.hpp"

#define INVALID_PLAYER_ID (4)

class Player : public AnimatedEntity
{
public:
	//-----Public Methods-----

	// Initialization
	Player(unsigned int playerID);
	~Player();

	// Core Loop
	void			ProcessGameplayInput();
	virtual void	Update() override;

	// Events
	virtual void	OnEntityCollision(Entity* other) override;
	virtual void	OnDamageTaken(int damageAmount) override;
	virtual void	OnDeath() override;
	virtual void	OnSpawn() override;

	// Behavior
	void Shoot();

	// Mutators
	void Respawn();

	// Items
	void AddItemSet(const ItemSet_t& itemsToAdd);


private:
	//-----Private Methods-----

	void DebugDrawState() const;


private:
	//-----Private Data-----

	int		m_playerID = INVALID_PLAYER_ID;

	ItemSet_t m_items;

};