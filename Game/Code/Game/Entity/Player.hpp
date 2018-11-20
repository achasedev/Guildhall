/************************************************************************/
/* File: Player.hpp
/* Author: Andrew Chase
/* Date: September 22nd, 2017
/* Description: Class to represent a player entity
/************************************************************************/
#pragma once
#include "Game/Entity/AnimatedEntity.hpp"
#include "Engine/Core/Rgba.hpp"


class Player : public AnimatedEntity
{
public:
	//-----Public Methods-----

	// Initialization
	Player(int playerID);
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

	// Accessors
	Rgba GetPlayerColor() const;
	int GetPlayerID() const;


	// Items
	void AddItemSet(const ItemSet_t& itemsToAdd);

	// Utility
	static Rgba GetColorForPlayerID(int id);


private:
	//-----Private Methods-----

	void DebugDrawState() const;


private:
	//-----Private Data-----

	int m_playerID = -1;

	ItemSet_t m_items;
	Rgba m_color;

	static Rgba s_playerColors[4];

};