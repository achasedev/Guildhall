/************************************************************************/
/* File: EntityDefinition.hpp
/* Author: Andrew Chase
/* Date: November 13th, 2017
/* Bugs: None
/* Description: Class to define standard data for all entities
/************************************************************************/
#pragma once
#include <string>
#include "ThirdParty/tinyxml2/tinyxml2.h"
#include "Engine/Math/AABB2.hpp"

typedef tinyxml2::XMLElement XMLElement;
class SpriteAnimSetDef;

enum StatType
{
	STAT_ERROR = -1,
	STAT_ATTACK,
	STAT_DEFENSE,
	STAT_SPEED,
	NUM_STAT_TYPES
};

struct Stats
{
	int m_stats[NUM_STAT_TYPES];

	const Stats operator+( const Stats& vecToAdd ) const
	{
		Stats result;
		for (int i = 0; i < NUM_STAT_TYPES; i++)
		{
			result.m_stats[i] = m_stats[i] + vecToAdd.m_stats[i];
		}

		return result;
	}
};


class EntityDefinition 
{
public:
	friend class Entity;

	//-----Public Methods-----

	EntityDefinition(const XMLElement& entityDefElement);

	//-----Accessors-----
	std::string					GetName() const;
	std::string					GetFaction() const;
	int							GetMaxHealth() const;
	int							GetStartHealth() const;
	float						GetInnerRadius() const;
	AABB2						GetLocalDrawBounds() const;

	bool						CanSee() const;
	bool						CanWalk() const;
	bool						CanFly() const;
	bool						CanSwim() const;
	float						GetViewDistance() const;
	float						GetWalkSpeed() const;
	float						GetFlySpeed() const;
	float						GetSwimSpeed() const;

	const SpriteAnimSetDef*		GetAnimSetDefinition() const;


private:
	//-----Private Methods-----

	void ParseSizeData(const XMLElement& entityDefElement);			// Initializes the radius and draw bounds data from the MovementInfo element
	void ParseHealthData(const XMLElement& entityDefElement);		// Initializes the health-related data members
	void ParseStatData(const XMLElement& entityDefElement);			// Initializes the min/max stats for this definition
	void ParseMovementData(const XMLElement& entityDefElement);		// Initializes the movement data of this definition
	void ParseAnimationData(const XMLElement& entityDefElement);	// Constructs the set of animations from the child XMLElement


private:
	//-----Private Data-----

	// Naming
	std::string		m_name = "";
	std::string		m_faction = "";

	// Health/Stats
	int				m_maxHealth	= 0;
	int				m_startHealth = 0;
	Stats			m_minBaseStats;	// Min possible base stats of this entity
	Stats			m_maxBaseStats;	// Max possible base stats for this entity

	// Size
	float			m_innerRadius = 0.4f;	// Physics radius of the object
	AABB2			m_localDrawBounds;		// The draw bounds for this entity definition	

	// Movement - negative values means it cannot see/walk/fly/swim, 0.f means it can occupy those tiles but not move
	float			m_viewDistance	= -1.0f;
	float			m_walkSpeed		= -1.0f;
	float			m_flySpeed		= -1.0f;
	float			m_swimSpeed		= -1.0f;

	// Animation/Rendering
	SpriteAnimSetDef*	m_animationSetDef = nullptr;	// Collection of animation definitions for this entity definition
};
