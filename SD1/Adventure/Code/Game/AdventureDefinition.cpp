/************************************************************************/
/* File: AdventureDefinition.cpp
/* Author: Andrew Chase
/* Date: November 19th, 2017
/* Bugs: None
/* Description: Implementation of the AdventureDefinition class
/************************************************************************/
#include "Game/AdventureDefinition.hpp"
#include "Game/ActorDefinition.hpp"
#include "Game/TileDefinition.hpp"
#include "Game/PortalDefinition.hpp"
#include "Engine/Core/XmlUtilities.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

// Static map of all actor definitions loaded
std::map<std::string, AdventureDefinition*> AdventureDefinition::s_definitions;


//-----------------------------------------------------------------------------------------------
// Constructs an AdventureDefinition from an XML element
//
AdventureDefinition::AdventureDefinition(const XMLElement& adventureDefElement)
{
	// Parse attributes on the Definition element first
	m_name	= ParseXmlAttribute(adventureDefElement, "name", nullptr);
	m_title = ParseXmlAttribute(adventureDefElement, "title", nullptr);

	// Parse child data
	ParseStartConditions(adventureDefElement);
	ParseVictoryConditions(adventureDefElement);
	ParseMapGenerationData(adventureDefElement);
}


//-----------------------------------------------------------------------------------------------
// Loads the Adventure XML file and constructs all AdventureDefinitions
//
void AdventureDefinition::LoadDefinitions()
{
	// Load the XML file, and parse the root-child elements for tile definitions
	tinyxml2::XMLDocument adventureDefinitionDocument;
	adventureDefinitionDocument.LoadFile("Data/Definitions/AdventureDefinition.xml");

	tinyxml2::XMLElement* currDefinitionElement = adventureDefinitionDocument.RootElement()->FirstChildElement();

	while (currDefinitionElement != nullptr)
	{
		AdventureDefinition* newDefinition = new AdventureDefinition(*currDefinitionElement);

		// Safety check - no duplicate definitions
		bool definitionAlreadyMade = (s_definitions.find(newDefinition->GetName()) != s_definitions.end());
		GUARANTEE_OR_DIE(!definitionAlreadyMade, Stringf("Error: Duplicate Adventure definition in AdventureDefinition.xml - \"%s\"", newDefinition->GetName().c_str()));

		// Add the definition to the map
		s_definitions[newDefinition->GetName()] = newDefinition;

		// Move to the next element
		currDefinitionElement = currDefinitionElement->NextSiblingElement();
	}
}


//-----------------------------------------------------------------------------------------------
// Returns the AdventureDefinition given by definitionName, if it exists
//
const AdventureDefinition* AdventureDefinition::GetDefinitionByName(const std::string& definitionName)
{
	// Safety check - check if the adventure exists
	bool definitionExists = (s_definitions.find(definitionName) != s_definitions.end());
	GUARANTEE_OR_DIE(definitionExists, Stringf("Error: AdventureDefinition::GetDefinitionByName could not find definition named \"%s\"", definitionName.c_str()));

	return s_definitions.at(definitionName);
}


//-----------------------------------------------------------------------------------------------
// Returns the name of this adventure definition
//
std::string AdventureDefinition::GetName() const
{
	return m_name;
}


//-----------------------------------------------------------------------------------------------
// Parses the start conditions element data from an XMLElement
//
void AdventureDefinition::ParseStartConditions(const XMLElement& adventureDefElement)
{
	const XMLElement* startElement = adventureDefElement.FirstChildElement("StartConditions");

	if (startElement != nullptr)
	{
		m_startMapName = ParseXmlAttribute(*startElement, "startMap", nullptr);
		m_startTileType = TileDefinition::ParseXMLAttribute(*startElement, "startTileType", nullptr);
	}

	// Ensure the Tile Definition is valid
	GUARANTEE_OR_DIE(m_startTileType != nullptr, Stringf("Error: AdventureDefinition::ParseStartConditions did not specify a start tile type."));
}


//-----------------------------------------------------------------------------------------------
// Parses the victory conditions element data from an XMLElement
//
void AdventureDefinition::ParseVictoryConditions(const XMLElement& adventureDefElement)
{
	const XMLElement* victoryElement = adventureDefElement.FirstChildElement("VictoryConditions");

	if (victoryElement != nullptr)
	{
		m_nameOfEnemyToDefeat = ParseXmlAttribute(*victoryElement, "haveDied", nullptr);
	}
}


//-----------------------------------------------------------------------------------------------
// Parses the map data for an adventure from a series of XMLElements
//
void AdventureDefinition::ParseMapGenerationData(const XMLElement& adventureDefElement)
{
	const XMLElement* mapElement = adventureDefElement.FirstChildElement("Map");
	
	while (mapElement != nullptr)
	{
		// Get parent map data first
		MapToGenerate* currMapToGenerate = new MapToGenerate();
		currMapToGenerate->m_name = ParseXmlAttribute(*mapElement, "name", nullptr);
		currMapToGenerate->m_mapDef = MapDefinition::ParseXMLAttribute(*mapElement, "mapDefinition", nullptr);

		// Ensure the map definition is valid
		GUARANTEE_OR_DIE(currMapToGenerate->m_mapDef != nullptr, Stringf("Error: AdventureDefinition::ParseMapsToGenerateData had map \"%s\" with no mapDefinition.", currMapToGenerate->m_name.c_str()));
	
		// Get the individual map data from its child elements
		ParseMapActorData(currMapToGenerate, *mapElement);
		ParseMapPortalData(currMapToGenerate, *mapElement);

		// Add the MapToGenerate element to this AdventureDefinition's list of them and iterate
		m_mapsToGenerate.push_back(currMapToGenerate);
		mapElement = mapElement->NextSiblingElement("Map");
	}
}


//-----------------------------------------------------------------------------------------------
// Parses the map actor data for a given map from an XMLElement
//
void AdventureDefinition::ParseMapActorData(MapToGenerate* mapToGenStruct, const XMLElement& mapElement)
{
	const XMLElement* actorElement = mapElement.FirstChildElement("Actor");

	while (actorElement != nullptr)
	{
		ActorToSpawn* newActorToSpawn = new ActorToSpawn();

		// Actor definition
		newActorToSpawn->m_actorDef = ActorDefinition::ParseXMLAttribute(*actorElement, "type", nullptr);
		GUARANTEE_OR_DIE(newActorToSpawn->m_actorDef != nullptr, Stringf("Error: AdventureDefinition::ParseMapActorData had actor with no actorDefinition, map was \"%s\"", mapToGenStruct->m_name.c_str()));

		// Spawn tile definition
		newActorToSpawn->m_tileDefToSpawnOn = TileDefinition::ParseXMLAttribute(*actorElement, "spawnTileType", nullptr);
		GUARANTEE_OR_DIE(newActorToSpawn->m_tileDefToSpawnOn != nullptr, Stringf("Error: AdventureDefinition::ParseMapActorData had actor with no spawnTileType, map was \"%s\"", mapToGenStruct->m_name.c_str()));

		newActorToSpawn->m_actorName = ParseXmlAttribute(*actorElement, "name", newActorToSpawn->m_actorName);

		// Add the ActorToSpawn to the list
		mapToGenStruct->m_actorsToSpawn.push_back(newActorToSpawn);

		// Iterate
		actorElement = actorElement->NextSiblingElement("Actor");
	}
}


//-----------------------------------------------------------------------------------------------
// Parses the map portal data for a given map from an XMLElement
//
void AdventureDefinition::ParseMapPortalData(MapToGenerate* mapToGenStruct, const XMLElement& mapElement)
{
	const XMLElement* portalElement = mapElement.FirstChildElement("Portal");

	while (portalElement != nullptr)
	{
		PortalToSpawn* newPortalToSpawn = new PortalToSpawn();

		// Portal type
		newPortalToSpawn->m_portalDef = PortalDefinition::ParseXMLAttribute(*portalElement, "type", nullptr);
		GUARANTEE_OR_DIE(newPortalToSpawn->m_portalDef != nullptr, Stringf("Error: AdventureDefinition::ParseMapPortalData had portal with no portalType, map was \"%s\"", mapToGenStruct->m_name.c_str()));

		// Reciprocal type
		newPortalToSpawn->m_reciprocalPortalType = PortalDefinition::ParseXMLAttribute(*portalElement, "reciprocalType", nullptr);	// This can be nullptr - just means no return portal
	
		// Spawn tile type
		newPortalToSpawn->m_tileDefToSpawnOn = TileDefinition::ParseXMLAttribute(*portalElement, "onTileType", nullptr);
		GUARANTEE_OR_DIE(newPortalToSpawn->m_tileDefToSpawnOn != nullptr, Stringf("Error: AdventureDefinition::ParseMapPortalData had portal with no onTileType, map was \"%s\"", mapToGenStruct->m_name.c_str()));

		// Destination tile type
		newPortalToSpawn->m_tileDefToTeleportTo = TileDefinition::ParseXMLAttribute(*portalElement, "toTileType", nullptr);
		GUARANTEE_OR_DIE(newPortalToSpawn->m_tileDefToTeleportTo != nullptr, Stringf("Error: AdventureDefinition::ParseMapPortalData had portal with no toTileType, map was \"%s\"", mapToGenStruct->m_name.c_str()));

		// Destination map name
		newPortalToSpawn->m_destinationMapName = ParseXmlAttribute(*portalElement, "toMap", nullptr);
		GUARANTEE_OR_DIE((newPortalToSpawn->m_destinationMapName.compare("") != 0), Stringf("Error: AdventureDefinition::ParseMapPortalData had no destination map specified, spawn map was \"%s\"", mapToGenStruct->m_name.c_str()));

		// Add to the list and iterate
		mapToGenStruct->m_portalsToSpawn.push_back(newPortalToSpawn);
		portalElement = portalElement->NextSiblingElement("Portal");
	}
}
