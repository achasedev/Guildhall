/************************************************************************/
/* File: LootTable.cpp
/* Author: Andrew Chase
/* Date: March 14th, 2019
/* Description: Implementation of the LootTable class
/************************************************************************/
#include "Game/Framework/LootTable.hpp"
#include "Game/Entity/EntityDefinition.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Utility/StringUtils.hpp"
#include "Engine/Core/Utility/XmlUtilities.hpp"
#include "Engine/Core/Utility/ErrorWarningAssert.hpp"
#include "Engine/Core/DeveloperConsole/DevConsole.hpp"

std::map<std::string, const LootTable*> LootTable::s_lootTables;

//-----------------------------------------------------------------------------------------------
// Loads the single LootTables file from disc and constructs and stores all LootTables
//
void LootTable::LoadTables(const std::string& xmlPath)
{
	XMLDocument document;
	XMLError error = document.LoadFile(xmlPath.c_str());
	GUARANTEE_OR_DIE(error == tinyxml2::XML_SUCCESS, "Couldn't open LootTables data file");

	const XMLElement* rootElement = document.RootElement();
	GUARANTEE_OR_DIE(rootElement != nullptr, "LootTable file doesn't have a root element");

	const XMLElement* tableElement = rootElement->FirstChildElement("LootTable");
	GUARANTEE_OR_DIE(tableElement != nullptr, "LootTable file doesn't have any tables listed");

	while (tableElement != nullptr)
	{
		LootTable* lootTable = new LootTable();
		lootTable->m_name = ParseXmlAttribute(*tableElement, "name", lootTable->m_name);
		lootTable->m_baseChanceToDropAnything = ParseXmlAttribute(*tableElement, "chance", lootTable->m_baseChanceToDropAnything);
		
		// Get the weapon drops
		const XMLElement* weaponElement = tableElement->FirstChildElement("Weapon");
		GUARANTEE_OR_DIE(weaponElement != nullptr, "LootTable doesn't have any weapons in it");

		while (weaponElement != nullptr)
		{
			std::string weaponName = ParseXmlAttribute(*weaponElement, "name", "");
			GUARANTEE_OR_DIE(!IsStringNullOrEmpty(weaponName), "No weapon name specified in LootTable");
			GUARANTEE_OR_DIE(EntityDefinition::GetDefinition(weaponName) != nullptr, "LootTable has weapon specified that doesn't exist");
			
			float chance = ParseXmlAttribute(*weaponElement, "chance", 1.0f);

			WeaponDrop_t drop;
			drop.weaponName = weaponName;
			drop.chance = chance;
			
			lootTable->m_weaponDrops.push_back(drop);

			weaponElement = weaponElement->NextSiblingElement("Weapon");
		}

		AddLootTableToRegistry(lootTable);
		tableElement = tableElement->NextSiblingElement("LootTable");
	}
}


//-----------------------------------------------------------------------------------------------
// Returns the table given by name, nullptr if it doesn't exist
//
const LootTable* LootTable::GetTableByName(const std::string& tableName)
{
	bool tableExists = s_lootTables.find(tableName) != s_lootTables.end();

	if (tableExists)
	{
		return s_lootTables.at(tableName);
	}

	ConsoleWarningf("LootTable %s doesn't exist", tableName.c_str());
	return nullptr;
}


//-----------------------------------------------------------------------------------------------
// Returns the definition of the weapon to drop. Returns nullptr if there is no weapon to drop (failed chance)
//
const EntityDefinition* LootTable::GetWeaponDrop() const
{
	// Check if we should drop anything at all this time
	if (!CheckRandomChance(m_baseChanceToDropAnything))
	{
		return nullptr;
	}

	// Find the weapon we should drop
	float weaponRoll = GetRandomFloatZeroToOne();

	int weaponCount = (int)m_weaponDrops.size();

	for (int weaponIndex = 0; weaponIndex < weaponCount; ++weaponIndex)
	{
		WeaponDrop_t currDrop = m_weaponDrops[weaponIndex];

		if (weaponRoll < currDrop.chance)
		{
			// We already checked that all weapons exist on file load, so this never returns nullptr
			return EntityDefinition::GetDefinition(currDrop.weaponName);
		}
		
		weaponRoll -= currDrop.chance;
	}

	// If the total chances of all drops sum to 1 as they should, we will never get here
	return nullptr;
}


//-----------------------------------------------------------------------------------------------
// Adds the loot table to the registry, checking for duplicates
//
void LootTable::AddLootTableToRegistry(const LootTable* lootTable)
{
	bool alreadyExists = s_lootTables.find(lootTable->m_name) != s_lootTables.end();
	GUARANTEE_OR_DIE(!alreadyExists, Stringf("Duplicate loot table %s", lootTable->m_name.c_str()).c_str());

	s_lootTables[lootTable->m_name] = lootTable;
}