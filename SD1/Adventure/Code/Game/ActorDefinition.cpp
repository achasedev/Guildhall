/************************************************************************/
/* File: ActorDefinition.cpp
/* Author: Andrew Chase
/* Date: November 13th, 2017
/* Bugs: None
/* Description: Implementation of the ActorDefinition class
/************************************************************************/
#include "Game/ActorDefinition.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/XmlUtilities.hpp"

// Static map of all actor definitions loaded
std::map<std::string, ActorDefinition*> ActorDefinition::s_definitions;


//-----------------------------------------------------------------------------------------------
// Constructs from an XMLElement
//
ActorDefinition::ActorDefinition(const tinyxml2::XMLElement& actorDefElement)
	: EntityDefinition(actorDefElement)
{
	ParseEquipmentData(actorDefElement);
	ParseBehaviorData(actorDefElement);
}


//-----------------------------------------------------------------------------------------------
// Loads the ActorDefinition XML file and constructs all ActorDefinitions
//
void ActorDefinition::LoadDefinitions()
{
	// Load the XML file, and parse the root-child elements for tile definitions
	tinyxml2::XMLDocument actorDefinitionDocument;
	actorDefinitionDocument.LoadFile("Data/Definitions/ActorDefinition.xml");

	tinyxml2::XMLElement* currDefinitionElement = actorDefinitionDocument.RootElement()->FirstChildElement();

	while (currDefinitionElement != nullptr)
	{
		ActorDefinition* newDefinition = new ActorDefinition(*currDefinitionElement);

		// Safety check - no duplicate definitions
		bool definitionAlreadyMade = (s_definitions.find(newDefinition->GetName()) != s_definitions.end());
		GUARANTEE_OR_DIE(!definitionAlreadyMade, Stringf("Error: Duplicate Actor definition in ActorDefinition.xml - \"%s\"", newDefinition->GetName().c_str()));

		// Add the definition to the map
		s_definitions[newDefinition->GetName()] = newDefinition;

		// Move to the next element
		currDefinitionElement = currDefinitionElement->NextSiblingElement();
	}
}


//-----------------------------------------------------------------------------------------------
// Returns the actor definition given by definitionName if it exists
//
const ActorDefinition* ActorDefinition::GetDefinitionByName(const std::string& definitionName)
{
	// Safety check - ensure the definition exists
	bool definitionExists = (s_definitions.find(definitionName) != s_definitions.end());
	GUARANTEE_OR_DIE(definitionExists, Stringf("Error: ActorDefinition::GetDefinitionByName - \"%s\" does not exist.", definitionName.c_str()));

	return s_definitions[definitionName];
}


//-----------------------------------------------------------------------------------------------
// Parses the given XML element for the equipment data associated with this actor definition
//
void ActorDefinition::ParseEquipmentData(const XMLElement& actorDefElement)
{
	const XMLElement* equipmentElement = actorDefElement.FirstChildElement("Equipment");

	if (equipmentElement != nullptr)
	{
		// Head
		const XMLElement* headElement = equipmentElement->FirstChildElement("Head");

		if (headElement != nullptr)
		{
			std::string headTypeName = ParseXmlAttribute(*headElement, "type", nullptr);
			m_spawnEquipment[EQUIP_SLOT_HEAD] = ItemDefinition::GetDefinitionByName(headTypeName);
		}

		// Chest
		const XMLElement* chestElement = equipmentElement->FirstChildElement("Chest");

		if (chestElement != nullptr)
		{
			std::string chestTypeName = ParseXmlAttribute(*chestElement, "type", nullptr);
			m_spawnEquipment[EQUIP_SLOT_CHEST] = ItemDefinition::GetDefinitionByName(chestTypeName);
		}

		// Arms
		const XMLElement* armsElement = equipmentElement->FirstChildElement("Arms");

		if (armsElement != nullptr)
		{
			std::string armsTypeName = ParseXmlAttribute(*armsElement, "type", nullptr);
			m_spawnEquipment[EQUIP_SLOT_ARMS] = ItemDefinition::GetDefinitionByName(armsTypeName);
		}

		// Hands
		const XMLElement* handsElement = equipmentElement->FirstChildElement("Hands");

		if (handsElement != nullptr)
		{
			std::string handsTypeName = ParseXmlAttribute(*handsElement, "type", nullptr);
			m_spawnEquipment[EQUIP_SLOT_HANDS] = ItemDefinition::GetDefinitionByName(handsTypeName);
		}

		// Legs
		const XMLElement* legsElement = equipmentElement->FirstChildElement("Legs");

		if (legsElement != nullptr)
		{
			std::string legsTypeName = ParseXmlAttribute(*legsElement, "type", nullptr);
			m_spawnEquipment[EQUIP_SLOT_LEGS] = ItemDefinition::GetDefinitionByName(legsTypeName);
		}

		// Feet
		const XMLElement* feetElement = equipmentElement->FirstChildElement("Feet");

		if (feetElement != nullptr)
		{
			std::string feetTypeName = ParseXmlAttribute(*feetElement, "type", nullptr);
			m_spawnEquipment[EQUIP_SLOT_FEET] = ItemDefinition::GetDefinitionByName(feetTypeName);
		}

		// Spear
		const XMLElement* bowElement = equipmentElement->FirstChildElement("Bow");

		if (bowElement != nullptr)
		{
			std::string bowTypeName = ParseXmlAttribute(*bowElement, "type", nullptr);
			m_spawnEquipment[EQUIP_SLOT_BOW] = ItemDefinition::GetDefinitionByName(bowTypeName);
		}

		// Bow
		const XMLElement* spearElement = equipmentElement->FirstChildElement("Spear");

		if (spearElement != nullptr)
		{
			std::string spearTypeName = ParseXmlAttribute(*spearElement, "type", nullptr);
			m_spawnEquipment[EQUIP_SLOT_SPEAR] = ItemDefinition::GetDefinitionByName(spearTypeName);
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Parses the given element for the behavior names and stores them in the definition
//
void ActorDefinition::ParseBehaviorData(const XMLElement& actorDefElement)
{
	const XMLElement* behaviorsElement = actorDefElement.FirstChildElement("Behaviors");

	if (behaviorsElement != nullptr)
	{
		const XMLElement* currBehaviorElement = behaviorsElement->FirstChildElement();

		while (currBehaviorElement != nullptr)
		{
			m_behaviorNames.push_back(currBehaviorElement->Name());
			currBehaviorElement = currBehaviorElement->NextSiblingElement();
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Parses the given XML element for the given attribute and interprets it as the name of an ActorDefinition,
// returning the corresponding ActorDefinition* from the static array
//
const ActorDefinition* ActorDefinition::ParseXMLAttribute(const XMLElement& element, const char* attributeName, ActorDefinition* defaultValue)
{
	std::string defaultActorName = ParseXmlAttribute(element, attributeName, nullptr);

	if (defaultActorName.compare(std::string("")) == 0)
	{
		return defaultValue;
	}

	// Safety check - ensure the definition exists
	bool definitionExists = (s_definitions.find(defaultActorName) != s_definitions.end());
	GUARANTEE_OR_DIE(definitionExists, Stringf("Error: ActorDefinition::ParseXMLAttribute does not have ActorDefinition \"%s\".", defaultActorName.c_str()));

	return s_definitions[defaultActorName];
}
