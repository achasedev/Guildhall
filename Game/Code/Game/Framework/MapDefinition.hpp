/************************************************************************/
/* File: MapDefinition.hpp
/* Author: Andrew Chase
/* Date: January 29th, 2018
/* Description: Class to represent the definition of a given map type
/************************************************************************/
#include "Game/Entity/EntitySpawn.hpp"
#include <map>
#include <string>

class MapDefinition
{
public:
	//-----Public Methods-----

	static void		LoadMap(const std::string& mapFilePath);
	static const	MapDefinition* GetDefinitionByName(const std::string& mapName);
	

public:
	//-----Public Data-----

	std::string m_name;

	std::string m_terrainType; // For randomization of terrains
	std::string m_terrainName; // For specific terrains (if specified)

	float m_gravityScale = 1.0f;

	std::vector<MapAreaSpawn_t> m_initialSpawns; // For spawning entities on the map when it's initialized


private:
	//-----Private Methods-----

	static void AddDefinitionToRegistry(const MapDefinition* definition);


private:
	//-----Private Data-----

	static std::map<std::string, const MapDefinition*> s_definitions;

};