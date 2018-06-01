/************************************************************************/
/* File: Actor.hpp
/* Author: Andrew Chase
/* Date: February 22nd, 2017
/* Description: Class to represent a single agent in the game
/************************************************************************/
#pragma once
#include <string>
#include <vector>
#include "Game/IsoSprite.hpp"
#include "Game/PlayAction.hpp"
#include "Engine/Math/IntVector2.hpp"
#include "Engine/Math/IntVector3.hpp"


class Animator;
class BoardState;
class ActorController;
class ActorDefinition;

class Actor
{
public:
	//-----Public Methods-----

	Actor(const std::string& actorName, ActorDefinition* definition, const IntVector2& spawnCoords, int teamIndex = 0);
	~Actor();

	void Update();
	void RenderSprite() const;
	void RenderHealthBar() const;

	//-----Accessors-----
	
	// From Actor itself
	Animator*	GetAnimator() const;
	float		GetWaitTime() const;
	std::string GetActorName() const;
	int			GetTeamIndex() const;
	float		GetOrientation() const ;
	int			GetCurrentHealth() const;
	Vector3		GetWorldPosition() const;
	int			GetTurnsSinceLastAction() const;

	// From ActorDefinition
	int			GetStrength() const;
	int			GetMaxHealth() const;
	int			GetMoveSpeed() const;
	float		GetBlockRate() const;
	float		GetCritChance() const;
	int			GetJumpHeight() const;
	float		GetActionSpeed() const;
	int			GetActorHeight() const;
	std::string GetDefinitionName() const;

	const std::vector<eActionName>& GetActions() const;


	//-----Mutators-----
	void		TakeDamage(int damageAmount);
	void		SetOrientation(float degrees);
	void		SetWorldPosition(const Vector3& position);

	void SetWaitTime(float waitTime);
	void AddToWaitTime(float amountToAdd);

	void ClearTurnCount();
	void SetTurnCount(int count);
	void IncrementTurnCount();

	void SetBlockRateBonus(float bonus);
	void AddToBlockRateBonus(float increment);
	void SetCritChanceBonus(float bonus);
	void AddToCritChanceBonus(float increment);

	//-----Producers-----
	bool		IsDead() const;
	Vector3		GetWorldForward() const;
	IntVector3	GetMapCoordinate() const;
	float		GetTimeUntilNextAction() const;


private:
	//-----Private Data-----

	// State
	std::string		m_name;						// Name of this specific actor
	int				m_health;					// Health of an actor, > 0 means alive
	float			m_waitTime;					// How long until this unit can act again
	int				m_teamIndex;				// Which team the actor is a part of
	Vector3			m_worldPosition;			// Absolute world position of the actor's feet
	float			m_bonusCritChance;			// Extra % chance for this actor to crit on an enemy
	float			m_bonusBlockChance;			// Extra % chance for this actor to block an attack
	int				m_numTurnsSinceLastAction;	// Used to break ties when deciding which actor to go next

	Animator*		m_animator;					// The object controlling this actor's animation rendering
	float			m_orientation;				// Degree value of this actor's facing direction (in the xz-plane)
	

	// Data
	ActorDefinition* m_definition;

};
