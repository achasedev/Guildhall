#include "Game/Framework/MapDefinition.hpp"
#include "Game/Entity/EntityDefinition.hpp"
#include "Engine/Core/Utility/StringUtils.hpp"
#include "Engine/Core/Utility/XmlUtilities.hpp"
#include "Engine/Core/Utility/ErrorWarningAssert.hpp"

// Registry of map definitions (only filled at boot up, not during gameplay)
std::map<std::string, const MapDefinition*> MapDefinition::s_definitions;

//-----------------------------------------------------------------------------------------------
// Loads a map from file and sets up the definition for it
//
void MapDefinition::LoadMap(const std::string& mapFilePath)
{
	// Load the file
	XMLDocument document;
	XMLError error = document.LoadFile(mapFilePath.c_str());
	GUARANTEE_RECOVERABLE(error == tinyxml2::XML_SUCCESS, Stringf("Couldn't load map file %s", mapFilePath.c_str()));

	// Get the root information
	const XMLElement* rootElement = document.RootElement();
	GUARANTEE_RECOVERABLE(rootElement != nullptr, Stringf("No root element specified in map %s", mapFilePath.c_str()));

	MapDefinition* mapDefinition = new MapDefinition();

	mapDefinition->m_name = ParseXmlAttribute(*rootElement, "name", mapDefinition->m_name);
	GUARANTEE_RECOVERABLE(!IsStringNullOrEmpty(mapDefinition->m_name), Stringf("No name specified for map %s", mapFilePath.c_str()));

	mapDefinition->m_terrainName = ParseXmlAttribute(*rootElement, "terrain", mapDefinition->m_terrainName);
	mapDefinition->m_terrainType = ParseXmlAttribute(*rootElement, "terrain_type", mapDefinition->m_terrainType);
	GUARANTEE_RECOVERABLE(!IsStringNullOrEmpty(mapDefinition->m_terrainType) || !IsStringNullOrEmpty(mapDefinition->m_terrainName), Stringf("No terrain type specified for map %s", mapFilePath.c_str()));

	// Iterate over all initial spawn elements and store off the spawn info
	// These spawns will be executed whenever a map is initialized from this definition
	const XMLElement* spawnElement = rootElement->FirstChildElement("Spawn");

	while (spawnElement != nullptr)
	{
		EntitySpawnArea_t spawnArea;

		// Definition to spawn
		std::string definitionName = ParseXmlAttribute(*spawnElement, "definition", definitionName);
		GUARANTEE_RECOVERABLE(!IsStringNullOrEmpty(definitionName), Stringf("No definition specified for spawn element in map %s", mapFilePath.c_str()));

		spawnArea.m_definitionToSpawn = EntityDefinition::GetDefinition(definitionName);

		// Spawn area
		spawnArea.m_spawnBoundsMins = ParseXmlAttribute(*spawnElement, "spawn_start", IntVector2(0, 0));
		IntVector2 spawnRange = ParseXmlAttribute(*spawnElement, "spawn_range", IntVector2(256, 256));

		spawnArea.m_spawnBoundsMaxs = spawnArea.m_spawnBoundsMins + spawnRange;

		// Spawn count and orientation
		spawnArea.m_spawnCount = ParseXmlAttribute(*spawnElement, "count", spawnArea.m_spawnCount);
		spawnArea.m_spawnOrientation = ParseXmlAttribute(*spawnElement, "orientation", spawnArea.m_spawnOrientation);

		// Overlap flags
		std::string overlapFlag = ParseXmlAttribute(*spawnElement, "overlap_type", "all");

		if (overlapFlag == "all")
		{
			spawnArea.m_allowOverlapsGlobally = false;
			spawnArea.m_allowOverlapsInThisArea = false;
		}
		else if (overlapFlag == "area")
		{
			spawnArea.m_allowOverlapsGlobally = true;
			spawnArea.m_allowOverlapsInThisArea = false;
		}
		else if (overlapFlag == "none")
		{
			spawnArea.m_allowOverlapsGlobally = true;
			spawnArea.m_allowOverlapsInThisArea = true;
		}
		else
		{
			ERROR_RECOVERABLE(Stringf("Invalid overlap flag \"%s\" specified in map file %s", overlapFlag.c_str(), mapFilePath.c_str()));

			spawnArea.m_allowOverlapsGlobally = false;
			spawnArea.m_allowOverlapsInThisArea = false;
		}

		// Add the spawn area to our list
		mapDefinition->m_initialSpawns.push_back(spawnArea);

		// Iterate
		spawnElement = spawnElement->NextSiblingElement("Spawn");
	}

	// Add it to the registry, checking for duplicates
	AddDefinitionToRegistry(mapDefinition);
}


//-----------------------------------------------------------------------------------------------
// Returns the definition given by the mapName from the registry, nullptr if it doesn't exist
//
const MapDefinition* MapDefinition::GetDefinitionByName(const std::string& mapName)
{
	bool exists = s_definitions.find(mapName) != s_definitions.end();

	if (exists)
	{
		return s_definitions.at(mapName);
	}

	return nullptr;
}


//-----------------------------------------------------------------------------------------------
// Adds the given definition to the registry, checking for duplicates
//
void MapDefinition::AddDefinitionToRegistry(const MapDefinition* definition)
{
	bool alreadyExists = s_definitions.find(definition->m_name) != s_definitions.end();
	GUARANTEE_OR_DIE(!alreadyExists, Stringf("Duplicate map definition exists for name \"%s\"", definition->m_name.c_str()));

	s_definitions[definition->m_name] = definition;
}
