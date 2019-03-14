/************************************************************************/
/* File: LootTable.hpp
/* Author: Andrew Chase
/* Date: March 14th, 2019
/* Description: Class for representing set of weapons an enemy can drop
/************************************************************************/
#include <map>
#include <string>
#include <vector>

class EntityDefinition;

struct WeaponDrop_t
{
	float chance = 1.0f;
	std::string weaponName;
};

class LootTable
{
public:
	//-----Public Methods-----

	const EntityDefinition* GetWeaponDrop() const;

	static void				LoadTables(const std::string& xmlPath);
	static const LootTable*	GetTableByName(const std::string& tableName);


private:
	//-----Private Methods-----

	LootTable() {}
	~LootTable() {}

	static void AddLootTableToRegistry(const LootTable* lootTable);


private:
	//-----Private Data-----

	std::string					m_name = "NO NAME SPECIFIED";
	float						m_baseChanceToDropAnything = 1.0f;
	std::vector<WeaponDrop_t>	m_weaponDrops;

	static std::map<std::string, const LootTable*> s_lootTables;

};
