/************************************************************************/
/* File: Player.hpp
/* Author: Andrew Chase
/* Date: September 22nd, 2017
/* Description: Class to represent a player entity
/************************************************************************/
#pragma once
#include "Game/Entity/AnimatedEntity.hpp"
#include "Engine/Core/Rgba.hpp"

class Weapon;
class Ability;

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
	void EquipWeapon(Weapon* weapon);
	void UnequipCurrentWeapon();
	void EquipAbility(Ability* ability);

	// Accessors
	Rgba GetPlayerColor() const;
	int GetPlayerID() const;
	Weapon* GetCurrentWeapon() const;

	// Utility
	static Rgba GetColorForPlayerID(int id);


private:
	//-----Private Data-----

	int m_playerID = -1;
	Rgba m_color;

	Weapon* m_currWeapon = nullptr;

	static Rgba s_playerColors[4];

};