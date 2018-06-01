/************************************************************************/
/* File: ActorDefinition.hpp
/* Author: Andrew Chase
/* Date: November 13th, 2017
/* Bugs: None
/* Description: Class to define actor-related data for actors
/************************************************************************/
#pragma once
#include <map>
#include <string>
#include <vector>
#include "Game/EntityDefinition.hpp"
#include "ThirdParty/tinyxml2/tinyxml2.h"
#include "Game/ItemDefinition.hpp"

class ActorDefinition : public EntityDefinition
{
public:
	friend class Actor;

	//-----Public Data-----

	ActorDefinition(const tinyxml2::XMLElement& actorDefElement);

	static void LoadDefinitions();
	static const ActorDefinition* GetDefinitionByName(const std::string& definitionName);
	static const ActorDefinition* ParseXMLAttribute(const XMLElement& element, const char* attributeName, ActorDefinition* defaultValue);


private:
	//-----Private Methods-----

	void ParseEquipmentData(const XMLElement& actorDefElement);
	void ParseBehaviorData(const XMLElement& actorDefElement);


private:
	//-----Private Data-----

	// Static list of definitions loaded from file
	static std::map<std::string, ActorDefinition*> s_definitions;
	const ItemDefinition* m_spawnEquipment[NUM_EQUIP_SLOTS];	// Equipment this actor spawns with

	std::vector<std::string> m_behaviorNames;					// Names of the behaviors this actor definition has
};
