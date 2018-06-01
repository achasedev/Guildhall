/************************************************************************/
/* File: ItemDefinition.hpp
/* Author: Andrew Chase
/* Date: November 28th, 2017
/* Bugs: None
/* Description: Class to represent an Item's definition data
/************************************************************************/
#pragma once
#include "Game/EntityDefinition.hpp"
#include <map>

typedef tinyxml2::XMLElement XMLElement;

// Slots that an item can fit on for an actor
enum EquipSlot
{
	EQUIP_SLOT_NONE = -1,
	EQUIP_SLOT_HEAD,
	EQUIP_SLOT_CHEST,
	EQUIP_SLOT_ARMS,
	EQUIP_SLOT_HANDS,
	EQUIP_SLOT_LEGS,
	EQUIP_SLOT_FEET,
	EQUIP_SLOT_NECK,
	EQUIP_SLOT_SPEAR,
	EQUIP_SLOT_SHIELD,
	EQUIP_SLOT_BOW,
	NUM_EQUIP_SLOTS
};


class ItemDefinition : public EntityDefinition
{
public:
	//-----Public Methods-----

	ItemDefinition(const XMLElement& itemDefElement);

	// Accessors
	EquipSlot	GetEquipSlot() const;

	// Static Methods
	static			void				LoadDefinitions();
	static const	ItemDefinition*		GetDefinitionByName(const std::string& definitionName);
	static			EquipSlot			ConvertStringToEquipSlot(const std::string& equipSlotName);
	static const	ItemDefinition*		ParseXMLAttribute(const XMLElement& element, const char* attributeName, ItemDefinition* defaultValue);


private:
	//-----Private Data-----

	EquipSlot m_equipSlot;				// Slot this Item equips to	

	// Static list of definitions loaded from file
	static std::map<std::string, ItemDefinition*> s_definitions; 

};
