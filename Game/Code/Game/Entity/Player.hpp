/************************************************************************/
/* File: Player.hpp
/* Author: Andrew Chase
/* Date: September 22nd, 2017
/* Description: Class to represent a player entity
/************************************************************************/
#pragma once
#include "Game/Entity/AnimatedEntity.hpp"
#include "Engine/Core/Rgba.hpp"
#include "Engine/Core/Time/Stopwatch.hpp"

class Weapon;
class Ability;

class Player : public AnimatedEntity
{
public:
	//-----Public Methods-----

	// Initialization
	Player(int playerID);
	Player(const EntityDefinition* definition, int playerID);
	~Player();

	void ReinitializeWithDefinition(const EntityDefinition* definition);

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

	// Accessors
	Rgba GetPlayerColor() const;
	int GetPlayerID() const;
	Weapon* GetCurrentWeapon() const;
	float GetRespawnTimeRemaining() const;
	bool IsRespawning() const;

	// Utility
	static Rgba GetColorForPlayerID(int id);


private:
	//-----Private Data-----

	int m_playerID = -1;
	Rgba m_color;

	Stopwatch m_respawnTimer;
	Weapon* m_currWeapon = nullptr;

	static constexpr float RESPAWN_INTERVAL = 5.0f;
	static Rgba s_playerColors[4];

};