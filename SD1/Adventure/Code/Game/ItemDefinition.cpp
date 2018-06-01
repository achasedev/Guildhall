/************************************************************************/
/* File: ItemDefinition.cpp
/* Author: Andrew Chase
/* Date: November 28th, 2017
/* Bugs: None
/* Description: Implementation of the ItemDefinition Class
/************************************************************************/
#include "Game/ItemDefinition.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/XmlUtilities.hpp"

// Static map of all actor definitions loaded
std::map<std::string, ItemDefinition*> ItemDefinition::s_definitions;


//-----------------------------------------------------------------------------------------------
// Constructor - Constructs from an XML element
//
ItemDefinition::ItemDefinition(const XMLElement& itemDefElement)
	: EntityDefinition(itemDefElement)
{
	std::string equipSlotName = ParseXmlAttribute(itemDefElement, "equipSlot", equipSlotName);
	m_equipSlot = ConvertStringToEquipSlot(equipSlotName);
}


//-----------------------------------------------------------------------------------------------
// Takes a string representation of a slot and returns the corresponding enumeration
//
EquipSlot ItemDefinition::ConvertStringToEquipSlot(const std::string& equipSlotName)
{
	if		(equipSlotName.compare("") == 0)		{ return EQUIP_SLOT_NONE;	}
	else if	(equipSlotName.compare("Head") == 0)	{ return EQUIP_SLOT_HEAD;	}
	else if (equipSlotName.compare("Chest") == 0)	{ return EQUIP_SLOT_CHEST;	}
	else if (equipSlotName.compare("Arms") == 0)	{ return EQUIP_SLOT_ARMS;	}
	else if (equipSlotName.compare("Hands") == 0)	{ return EQUIP_SLOT_HANDS;	}
	else if (equipSlotName.compare("Legs") == 0)	{ return EQUIP_SLOT_LEGS;	}
	else if (equipSlotName.compare("Feet") == 0)	{ return EQUIP_SLOT_FEET;	}
	else if (equipSlotName.compare("Neck") == 0)	{ return EQUIP_SLOT_NECK;	}
	else if (equipSlotName.compare("Bow") == 0)		{ return EQUIP_SLOT_BOW;	}
	else if (equipSlotName.compare("Spear") == 0)	{ return EQUIP_SLOT_SPEAR;  }
	else if (equipSlotName.compare("Shield") == 0)	{ return EQUIP_SLOT_SHIELD; }
	else
	{
		ERROR_AND_DIE(Stringf("Error: ItemDefinition::ConvertStringToEquipSlot received bad string \"%s\"", equipSlotName.c_str()));
	}
}


//-----------------------------------------------------------------------------------------------
// Parses the element for the given attribute and interprets it as an ItemDefinition
//
const ItemDefinition* ItemDefinition::ParseXMLAttribute(const XMLElement& element, const char* attributeName, ItemDefinition* defaultValue)
{
	std::string defaultItemName = ParseXmlAttribute(element, attributeName, nullptr);

	if (defaultItemName.compare(std::string("")) == 0)
	{
		return defaultValue;
	}

	// Safety check - ensure the definition exists
	bool definitionExists = (s_definitions.find(defaultItemName) != s_definitions.end());
	GUARANTEE_OR_DIE(definitionExists, Stringf("Error: ItemDefinition::ParseXMLAttribute does not have ItemDefinition \"%s\".", defaultItemName.c_str()));

	return s_definitions[defaultItemName];
}


//-----------------------------------------------------------------------------------------------
// Returns this item's equipslot
//
EquipSlot ItemDefinition::GetEquipSlot() const
{
	return m_equipSlot;
}


//-----------------------------------------------------------------------------------------------
// Loads the ItemDefinition XML file and constructs the definitions from it
//
void ItemDefinition::LoadDefinitions()
{
	// Load the XML file, and parse the root-child elements for tile definitions
	tinyxml2::XMLDocument itemDefinitionDocument;
	itemDefinitionDocument.LoadFile("Data/Definitions/ItemDefinition.xml");

	tinyxml2::XMLElement* currDefinitionElement = itemDefinitionDocument.RootElement()->FirstChildElement();

	while (currDefinitionElement != nullptr)
	{
		ItemDefinition* newDefinition = new ItemDefinition(*currDefinitionElement);

		// Safety check - no duplicate definitions
		bool definitionAlreadyMade = (s_definitions.find(newDefinition->GetName()) != s_definitions.end());
		GUARANTEE_OR_DIE(!definitionAlreadyMade, Stringf("Error: Duplicate ItemDefinition in ItemDefinition.xml - \"%s\"", newDefinition->GetName().c_str()));

		// Add the definition to the map
		s_definitions[newDefinition->GetName()] = newDefinition;

		// Move to the next element
		currDefinitionElement = currDefinitionElement->NextSiblingElement();
	}
}


//-----------------------------------------------------------------------------------------------
// Returns the definition given by definitionName, if it exists
//
const ItemDefinition* ItemDefinition::GetDefinitionByName(const std::string& definitionName)
{
	// Safety check - ensure the definition exists
	bool definitionExists = (s_definitions.find(definitionName) != s_definitions.end());
	GUARANTEE_OR_DIE(definitionExists, Stringf("Error: ItemDefinition::GetDefinitionByName - \"%s\" does not exist.", definitionName.c_str()));

	return s_definitions[definitionName];
}
