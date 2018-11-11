/************************************************************************/
/* File: Item.hpp
/* Author: Andrew Chase
/* Date: October 29th 2018
/* Description: Class to represent an item that can be picked up by the player
/************************************************************************/
#pragma once
#include "Game/Entity/Entity.hpp"
#include "Engine/Core/Time/Stopwatch.hpp"


class Item : public Entity
{
public:
	//-----Public Methods-----
	
	Item(const EntityDefinition* definition);

	virtual void					Update();

	virtual void					OnEntityCollision(Entity* other);
	virtual void					OnDeath();
	virtual void					OnSpawn();


public:
	//-----Public Data-----
	

private:
	//-----Private Methods-----
	
	
private:
	//-----Private Data-----

	Stopwatch m_bobTimer;

	Vector3 m_basePosition = Vector3::ZERO;

	static constexpr float ITEM_BOB_RATE = 180.f;
	static constexpr float ITEM_SPIN_RATE = 90.f;
};
