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
	virtual void		OnEntityCollision(Entity* other) override;
	void				OnEquip(Entity* playerEquipping);
	void				OnUnequip();
	void				Shoot();
	void				SetHasInfiniteAmmo(bool hasInfiniteAmmo);
	int					GetAmmoCountRemaining() const;
	bool				IsOutOfAmmo() const;
	const VoxelSprite*	GetTextureForUIRender();
	bool				IsFullAuto() const;


private:
	//-----Private Methods-----

	int CreateProjectiles(std::vector<Projectile*>& out_projectiles) const;
	int CreateProjectilesForFanSpread(std::vector<Projectile*>& out_projectiles) const;
	int CreateProjectilesForRandomSpread(std::vector<Projectile*>& out_projectiles) const;
	int CreateProjectilesForSourceSpread(std::vector<Projectile*>& out_projectiles) const;
	int CreateProjectilesForNoSpread(std::vector<Projectile*>& out_projectiles) const;


private:
	//-----Private Data-----

	Entity* m_entityEquippedTo = nullptr;
	Stopwatch m_shootTimer;
	int m_currAmmoCount = 0;
	bool m_hasInfiniteAmmo = false;

	// For rendering above players
	Stopwatch m_flashTimer;
	bool m_showTexture = true;

	// For rendering behavior, same for all weapons
	static constexpr float WEAPON_FIRE_OFFSET = 5.f;
	static constexpr float WEAPON_LOW_AMMO_PERCENTAGE = 0.2f;
	static constexpr float WEAPON_LOW_AMMO_FLASH_INTERVAL = 0.25f;

};
