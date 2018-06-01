/************************************************************************/
/* Project: Incursion
/* File: NpcTank.hpp
/* Author: Andrew Chase
/* Date: October 7th, 2017
/* Bugs: None
/* Description: Class to represent an AI-controlled moveable tank
/************************************************************************/
#pragma once
#include "Game/Entity.hpp"


// Enum of the different kinds of tanks
enum NpcTankType
{
	NPC_TANK_TYPE_ERROR = -1,
	NPC_TANK_TYPE_LIGHT,
	NPC_TANK_TYPE_MEDIUM,
	NPC_TANK_TYPE_HEAVY,
	NPC_TANK_TYPE_TD,
	NUM_NPC_TANK_TYPES
};


class NpcTank : public Entity
{
public:
	//-----Public Methods-----

	// Constructor for all NpcTanks, their types are chosen at random
	NpcTank(Vector2 spawnPosition, Faction entityFaction, NpcTankType tankType, Map* entityMap); 
	virtual ~NpcTank() override;

	virtual void Update(float deltaTime) override;
	virtual void Render() const;


private:
	//-----Private Methods-----

	void PerformAttackAI(float deltaTime);			// Behavior for the tank if the player is in range and line of sight
	void PerformPursueAI(float deltaTime);
	void PerformWanderAI(float deltaTime);			// Behavior for the tank if the player is not in range or out of line of sight
	bool PerformFollowAI(float deltaTime);			// Follow the player's trail of breadcrumbs, returns true if a breadcrumb is seen
	void ShootWithCooldown();						// Shoots a bullet out the front of the tank if the shoot cooldown is done

private:
	//-----Private Data-----

	float m_bulletCooldown;							// Time until the next bullet can be fired
	float m_timeUntilNewWanderTarget;				// Time until the tank chooses a new random wander orientation
	float m_wanderTargetOrientation;				// Random degree orientation the tank will drive towards if the player is out of sight
	float m_viewRange;								// The view range of this tank, in tile units
	float m_turnSpeed;								// The speed in degrees per second this tank can turn at
	float m_moveSpeed;								// the speed in tiles per second this tank can move at
	float m_fireRateScalar;							// How much faster/slower this tank can fire over the normal fire rate
	NpcTankType m_tankType;							// The type of tank this tank is

	Entity* m_target;								// Target entity this tank is pursuing
	Vector2 m_lastTargetPosition;					// Last known position of the target
	bool m_hasTargetPosition;
	float m_pursueTimer;							// Timer that tracks how long they've been attempting to pursue a target

	// Constants
	static const float NPC_TANK_BULLETS_PER_SECOND;			// Fire rate of this tank
	static const float NPC_TANK_DEFAULT_MOVE_SPEED;			// Default speed in tiles per second
	static const float NPC_TANK_DEFAULT_TURN_SPEED;			// Default turn speed in degrees per second
	static const int NPC_TANK_DEFAULT_HEALTH;				// Default entity health
	static const float NPC_TANK_DEFAULT_VIEW_RANGE;			// Default view range in tiles
	static const float NPC_MAX_PURSUE_DURATION;
	static const float NPC_TANK_MIN_DOT_TO_DRIVE_FORWARD;	// For target AI, minimum dot product for the tank to move forward towards the player
	static const float NPC_TANK_MIN_DOT_TO_SHOOT;			// For target AI, minimum dot product for the tank to shoot at the player
};
