/************************************************************************/
/* File: Weapon.hpp
/* Author: Andrew Chase
/* Date: November 20th 2018
/* Description: Class to represent a weapon that can be equipped by a player
/************************************************************************/
#pragma once
#include "Game/Entity/Item.hpp"

class Projectile;

class Weapon : public Item
{
public:
	//-----Public Methods-----
	
	Weapon(const EntityDefinition* definition);

	virtual void OnEntityCollision(Entity* other) override;

	void OnEquip(Player* playerEquipping);

	void Shoot();


private:
	//-----Private Data-----

	Player* m_playerEquippedTo = nullptr;
	Stopwatch m_shootTimer;

	static constexpr float WEAPON_FIRE_OFFSET = 5.f;

};
