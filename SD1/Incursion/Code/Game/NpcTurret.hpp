/************************************************************************/
/* Project: Incursion
/* File: NpcTurret.hpp
/* Author: Andrew Chase
/* Date: October 10th, 2017
/* Bugs: None
/* Description: Class to represent an immobile npc turret entity
/************************************************************************/
#pragma once
#include "Game/Entity.hpp"

class NpcTurret : public Entity
{
public:
	//-----Public Methods-----

	NpcTurret(Vector2 spawnPosition, Faction entityFaction, Map* entityMap);
	virtual ~NpcTurret() override;

	virtual void Update(float deltaTime) override;
	virtual void Render() const;

private:
	//-----Private Methods-----

	void PerformTargetAI(float deltaTime);		// Turns the gun towards the player and fires
	void PerformIdleBehavior(float deltaTime);	// Turns the turret back and forth

	Entity* FindTarget();
	void ShootWithCooldown();							// Shoots a bullet based on the turret's fire rate

private:
	//-----Private Data-----

	float m_bulletCooldown;						// Timer to limit the turret's fire rate

	Entity* m_target;
	float m_lastTargetOrientation;
	int m_idleTurnDirection;

	static const int NPC_TURRET_DEFAULT_HEALTH;		// Number of bullets it takes to destroy this turret
	static const float NPC_TURRET_DEFAULT_VIEW_RANGE;		// Number of tiles a turret can see the player from
	static const float NPC_TURRET_DEFAULT_TURN_SPEED;		// Degrees-per-second a turret can turn its gun
	static const float NPC_TURRET_BULLETS_PER_SECOND;		// Fire rate of the turret
	static const float NPC_TURRET_MIN_DOT_TO_SHOOT;		// For targeting, the minimum dot product necessary to shoot (turret's frontal cone)
	static const float NPC_TURRET_IDLE_TURN_LIMIT;		// Max number of degrees a turret will turn when idle
};