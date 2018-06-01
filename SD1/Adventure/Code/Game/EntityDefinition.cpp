/************************************************************************/
/* File: SpriteAnimSetDef.hpp
/* Author: Andrew Chase
/* Date: November 13th, 2017
/* Bugs: None
/* Description: Implementation of the SpriteAnimSetDef class
/************************************************************************/
#include "Game/GameCommon.hpp"
#include "Game/EntityDefinition.hpp"
#include "Engine/Core/XmlUtilities.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/SpriteAnimSetDef.hpp"
#include "Engine/Renderer/Renderer.hpp"


//-----------------------------------------------------------------------------------------------
// Constructor - parses an entity (sub)class XMLElement for the data and initializes accordingly
//
EntityDefinition::EntityDefinition(const XMLElement& entityDefElement)
{
	m_name		= ParseXmlAttribute(entityDefElement, "name",		m_name);
	m_faction	= ParseXmlAttribute(entityDefElement, "faction",	m_faction);

	ParseSizeData(entityDefElement);
	ParseHealthData(entityDefElement);
	ParseStatData(entityDefElement);
	ParseMovementData(entityDefElement);
	ParseAnimationData(entityDefElement);
}


//-----------------------------------------------------------------------------------------------
// Returns the name of this Entity Definition 
//
std::string EntityDefinition::GetName() const
{
	return m_name;
}


//-----------------------------------------------------------------------------------------------
// Returns the faction of this Entity Definition
//
std::string EntityDefinition::GetFaction() const
{
	return m_faction;
}


//-----------------------------------------------------------------------------------------------
// Returns the max health of this Entity Definition
//
int EntityDefinition::GetMaxHealth() const
{
	return m_maxHealth;
}


//-----------------------------------------------------------------------------------------------
// Returns the start health of this Entity Definition
//
int EntityDefinition::GetStartHealth() const
{
	return m_startHealth;
}


//-----------------------------------------------------------------------------------------------
// Returns the physics radius of this object
//
float EntityDefinition::GetInnerRadius() const
{
	return m_innerRadius;
}


//-----------------------------------------------------------------------------------------------
// Returns the draw bounds for this entity definition
//
AABB2 EntityDefinition::GetLocalDrawBounds() const
{
	return m_localDrawBounds;
}


//-----------------------------------------------------------------------------------------------
// Returns true if this Entity Definition can see (i.e. raycasting)
//
bool EntityDefinition::CanSee() const
{
	return (m_viewDistance >= 0.f);
}


//-----------------------------------------------------------------------------------------------
// Returns true if this Entity Definition can walk
//
bool EntityDefinition::CanWalk() const
{
	return (m_walkSpeed >= 0.f);
}


//-----------------------------------------------------------------------------------------------
// Returns true if this Entity Definition can fly
//
bool EntityDefinition::CanFly() const
{
	return (m_flySpeed >= 0.f);
}


//-----------------------------------------------------------------------------------------------
// Returns true if this Entity Definition can swim
//
bool EntityDefinition::CanSwim() const
{
	return (m_swimSpeed >= 0.f);
}


//-----------------------------------------------------------------------------------------------
// Returns the view distance of this entity
//
float EntityDefinition::GetViewDistance() const
{
	return m_viewDistance;
}


//-----------------------------------------------------------------------------------------------
// Returns the walk speed of this entity
//
float EntityDefinition::GetWalkSpeed() const
{
	return m_walkSpeed;
}


//-----------------------------------------------------------------------------------------------
// Returns the fly speed of this entity
//
float EntityDefinition::GetFlySpeed() const
{
	return m_flySpeed;
}


//-----------------------------------------------------------------------------------------------
// Returns the swim speed of this entity
//
float EntityDefinition::GetSwimSpeed() const
{
	return m_swimSpeed;
}


//-----------------------------------------------------------------------------------------------
// Returns the AnimationSetDefinition for this EntityDefinition
//
const SpriteAnimSetDef* EntityDefinition::GetAnimSetDefinition() const
{
	return m_animationSetDef;
}


//-----------------------------------------------------------------------------------------------
// Parses the entity definition element for the size child element and initializes the data members
//
void EntityDefinition::ParseSizeData(const XMLElement& entityDefElement)
{
	const tinyxml2::XMLElement* sizeElement = entityDefElement.FirstChildElement("Size");

	if (sizeElement != nullptr)
	{																													
		m_innerRadius		= ParseXmlAttribute(*sizeElement, "physicsRadius", m_innerRadius);
		m_localDrawBounds	= ParseXmlAttribute(*sizeElement, "localDrawBounds", m_localDrawBounds);
	}
}


//-----------------------------------------------------------------------------------------------
// Parses the entity definition element's health child element for health data and initializes the data members
//
void EntityDefinition::ParseHealthData(const XMLElement& entityDefElement)
{
	const tinyxml2::XMLElement* healthElement = entityDefElement.FirstChildElement("Health");

	if (healthElement != nullptr)
	{																													
		m_startHealth		= ParseXmlAttribute(*healthElement, "start", m_startHealth);
		m_maxHealth			= ParseXmlAttribute(*healthElement, "max", m_maxHealth);
	}
}


//-----------------------------------------------------------------------------------------------
// Parses the given XML element for the stat data associated with this actor definition
//
void EntityDefinition::ParseStatData(const XMLElement& entityDefElement)
{
	const XMLElement* statsElement = entityDefElement.FirstChildElement("Stats");

	if (statsElement != nullptr)
	{
		const XMLElement* minStatsElement = statsElement->FirstChildElement("Minimum");
		const XMLElement* maxStatsElement = statsElement->FirstChildElement("Maximum");

		// Minimum stats
		if (minStatsElement != nullptr)
		{
			// Parse the individual stats
			int defaultMinAttack		= m_minBaseStats.m_stats[STAT_ATTACK];
			int defaultMinDefense	= m_minBaseStats.m_stats[STAT_DEFENSE];
			int defaultMinSpeed		= m_minBaseStats.m_stats[STAT_SPEED];

			m_minBaseStats.m_stats[STAT_ATTACK]		= ParseXmlAttribute(*minStatsElement, "attack",		defaultMinAttack);
			m_minBaseStats.m_stats[STAT_DEFENSE]	= ParseXmlAttribute(*minStatsElement, "defense",	defaultMinDefense);		
			m_minBaseStats.m_stats[STAT_SPEED]		= ParseXmlAttribute(*minStatsElement, "speed",		defaultMinSpeed);		
		}

		// Maximum stats
		if (maxStatsElement != nullptr)
		{
			// Parse the individual stats
			int defaultMaxStrength		= m_maxBaseStats.m_stats[STAT_ATTACK];
			int defaultMaxIntelligence	= m_maxBaseStats.m_stats[STAT_DEFENSE];
			int defaultMaxDexterity		= m_maxBaseStats.m_stats[STAT_SPEED];

			m_maxBaseStats.m_stats[STAT_ATTACK]		= ParseXmlAttribute(*maxStatsElement, "attack",		defaultMaxStrength);
			m_maxBaseStats.m_stats[STAT_DEFENSE]	= ParseXmlAttribute(*maxStatsElement, "defense",	defaultMaxIntelligence);		
			m_maxBaseStats.m_stats[STAT_SPEED]		= ParseXmlAttribute(*maxStatsElement, "speed",		defaultMaxDexterity);		
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Parses the entity definition element's movement child element for movement data and initializes the data members
//
void EntityDefinition::ParseMovementData(const XMLElement& entityDefElement)
{
	const XMLElement* movementElement = entityDefElement.FirstChildElement("Movement");

	if (movementElement == nullptr) { return; }

	// Sight
	const XMLElement* sightElement = movementElement->FirstChildElement("Sight");
	if (sightElement != nullptr)
	{
		m_viewDistance = ParseXmlAttribute(*sightElement, "distance", m_viewDistance);
	}

	// Walk
	const XMLElement* walkElement = movementElement->FirstChildElement("Walk");
	if (walkElement != nullptr)
	{
		m_walkSpeed	= ParseXmlAttribute(*walkElement, "speed", m_walkSpeed);
	}

	// Fly
	const XMLElement* flyElement = movementElement->FirstChildElement("Fly");
	if (flyElement != nullptr)
	{
		m_flySpeed	= ParseXmlAttribute(*flyElement, "speed", m_flySpeed);
	}

	// Swim
	const XMLElement* swimElement = movementElement->FirstChildElement("Swim");
	if (swimElement != nullptr)
	{
		m_swimSpeed	= ParseXmlAttribute(*swimElement, "speed", m_swimSpeed);
	}
}


//-----------------------------------------------------------------------------------------------
// Parses the entity definition element for the animation set child element and initializes the data members
//
void EntityDefinition::ParseAnimationData(const tinyxml2::XMLElement& entityDefElement)
{
	const tinyxml2::XMLElement* animSetElement = entityDefElement.FirstChildElement("SpriteAnimSet");

	GUARANTEE_OR_DIE((animSetElement != nullptr), Stringf("Error: EntityDefintion::ParseAnimationData - No SpriteAnimSet element exists in XML for the definition."))
																													
	m_animationSetDef = new SpriteAnimSetDef(*animSetElement, *g_theRenderer);
}
