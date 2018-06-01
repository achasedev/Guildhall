/************************************************************************/
/* File: ActorDefinition.hpp
/* Author: Andrew Chase
/* Date: March 16th, 2018
/* Description: Class to represent Actor data loaded from XML
/************************************************************************/
#pragma once
#include <map>
#include <string>
#include "Game/PlayAction.hpp"
#include "Engine/Core/XmlUtilities.hpp"

class IsoSprite;
class AnimationSet;

class ActorDefinition
{
public:
	//-----Public Methods-----

	// Accessors
	std::string GetName() const;

	int		GetMaxHealth() const;
	int		GetMoveSpeed() const;
	int		GetJumpHeight() const;
	int		GetActorHeight() const;
	int		GetStrength() const;
	float	GetActionSpeed() const;
	float	GetBlockRate() const;
	float	GetCritChance() const;

	AnimationSet*	GetAnimationSet() const;
	IsoSprite*		GetDefaultSprite() const;

	const std::vector<eActionName>& GetActions() const;

	// Statics
	static void				LoadDefinitions(const std::string& filePath);
	static void				AddDefinition(ActorDefinition* definition);
	static ActorDefinition*	GetDefinition(const std::string& name);


private:
	//-----Private Methods-----

	// Only construct from LoadDefinitions()
	ActorDefinition(const XMLElement& defElement);


private:
	//-----Private Data-----

	// Definition name
	std::string	m_name;

	// Attributes
	int		m_maxHealth;				// Maximum health value this actor can have
	int		m_moveSpeed;				// How many tiles this actor can move per turn
	int		m_jumpHeight;				// Max height in blocks this actor can traverse between tiles
	int		m_height;					// How tall in blocks this actor is, used for targeting and rendering
	int		m_strength;					// How much damage this actor does when attacking
	float	m_critChance;				// Chance for the skill to do double damage
	float	m_blockRate;				// Chance to block the attack, dealing 0 damage
	float	m_actionSpeed;				// Rate at which time decays on this actor

	// Animation
	AnimationSet*	m_animationSet;
	IsoSprite*		m_defaultSprite;

	// Actions
	std::vector<eActionName>	m_actions;

	// Static map for all actor definitions
	static std::map<std::string, ActorDefinition*> s_definitions;

};
