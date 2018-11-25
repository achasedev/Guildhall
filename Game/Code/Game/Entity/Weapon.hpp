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

	// Events
	virtual void OnEntityCollision(Entity* other) override;
	void OnEquip(Player* playerEquipping);
	void OnUnequip();

	void Shoot();
	bool IsOutOfAmmo() const;

	const VoxelTexture* GetTextureForUIRender();


private:
	//-----Private Data-----

	Player* m_playerEquippedTo = nullptr;
	Stopwatch m_shootTimer;
	int m_currAmmoCount = 0;

	// For rendering above players
	Stopwatch m_flashTimer;
	bool m_showTexture = true;

	// For rendering behavior, same for all weapons
	static constexpr float WEAPON_FIRE_OFFSET = 5.f;
	static constexpr float WEAPON_LOW_AMMO_PERCENTAGE = 0.2f;
	static constexpr float WEAPON_LOW_AMMO_FLASH_INTERVAL = 0.25f;

};
