/************************************************************************/
/* File: Player.hpp
/* Author: Andrew Chase
/* Date: September 22nd, 2017
/* Description: Class to represent a player entity
/************************************************************************/
#pragma once
#include "Game/Entity/DynamicEntity.hpp"

class VoxelGrid;

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
	virtual void OnCollision(Entity* other) override;


private:
	//-----Private Data-----

	int m_playerID = 0;

};