/************************************************************************/
/* File: MapDefinition.cpp
/* Author: Andrew Chase
/* Date: November 2nd, 2017
/* Bugs: None
/* Description: Implementation of the MapDefinition class
/************************************************************************/
#include "Game/MapDefinition.hpp"
#include "Game/TileDefinition.hpp"
#include "Engine/Core/XmlUtilities.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"


std::map<std::string, MapDefinition*>	MapDefinition::s_definitions;


//-----------------------------------------------------------------------------------------------
// Explicit constructor - parses an XML Element representing a map definition and stores the object
// in the static vector of definitions
//
MapDefinition::MapDefinition(const tinyxml2::XMLElement& mapDefinitionElement)
{
	m_name		= ParseXmlAttribute(mapDefinitionElement, "name", m_name);
	m_width		= ParseXmlAttribute(mapDefinitionElement, "width", m_width);
	m_height	= ParseXmlAttribute(mapDefinitionElement, "height", m_height);

	// Safety check - valid map dimensions
	bool dimensionsInvalid = ((m_width.min <= 0) || (m_width.max <= 0) || (m_height.min <= 0) || (m_height.max <= 0));
	GUARANTEE_OR_DIE(!dimensionsInvalid, Stringf("Error: Map definition \"%s\" dimensions are invalid.", m_name.c_str()));

	// Get the default tile
	m_defaultTile = TileDefinition::ParseXMLAttribute(mapDefinitionElement, "defaultTile", m_defaultTile);
	GUARANTEE_OR_DIE(m_defaultTile != nullptr, Stringf("Error: MapDefinition::MapDefinition (constructor) - map \"%s\" has null default tile.", m_name.c_str()));

	// Create the generation steps
	const tinyxml2::XMLElement* generationStepsHeader = mapDefinitionElement.FirstChildElement("GenerationSteps");

	if (generationStepsHeader != nullptr)
	{
		const tinyxml2::XMLElement* currGenStepElement = generationStepsHeader->FirstChildElement();

		while (currGenStepElement != nullptr)
		{
			MapGenStep* currGenStep = MapGenStep::CreateMapGenStep(*currGenStepElement);
			m_generationSteps.push_back(currGenStep);
			currGenStepElement = currGenStepElement->NextSiblingElement();
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Returns the name of the definition
//
std::string MapDefinition::GetName() const
{
	return m_name;
}


//-----------------------------------------------------------------------------------------------
// Returns the default tile of this definition
//
const TileDefinition* MapDefinition::GetDefaultTile() const
{
	return m_defaultTile;
}


//-----------------------------------------------------------------------------------------------
// Returns the list of generators used by this definition when constructing a map
//
std::vector<MapGenStep*> MapDefinition::GetGenerators() const
{
	return m_generationSteps;
}


//-----------------------------------------------------------------------------------------------
// Returns an int value within the width range of this definition
//
int MapDefinition::GetRandomWidthInRange() const
{
	return m_width.GetRandomInRange();
}


//-----------------------------------------------------------------------------------------------
// Returns an int value within the height range of this definition
//
int MapDefinition::GetRandomHeightInRange() const
{
	return m_height.GetRandomInRange();
}


//-----------------------------------------------------------------------------------------------
// Loads the MapDefinition XML file from disk and constructs all the MapDefinitions from its contents
//
void MapDefinition::LoadDefinitions()
{
	// Load the XML file, and parse the root-child elements for tile definitions
	tinyxml2::XMLDocument tileDefinitionDocument;
	tileDefinitionDocument.LoadFile("Data/Definitions/MapDefinition.xml");

	tinyxml2::XMLElement* currDefinitionElement = tileDefinitionDocument.RootElement()->FirstChildElement();

	while (currDefinitionElement != nullptr)
	{
		// Add the definition to the map of definitions
		MapDefinition* newDefinition = new MapDefinition(*currDefinitionElement);

		// Safety check - no duplicate definitions
		bool definitionExists = (s_definitions.find(newDefinition->GetName()) != s_definitions.end());
		GUARANTEE_OR_DIE(!definitionExists, Stringf("Error: Duplicate Map definition in MapDefinition.xml - \"%s\"", newDefinition->GetName().c_str()));

		// Add the definition to the map
		s_definitions[newDefinition->GetName()] = newDefinition;

		// Move to the next element
		currDefinitionElement = currDefinitionElement->NextSiblingElement();
	}
}


//-----------------------------------------------------------------------------------------------
// Returns the map defintion associated with definitionName, if it exists
//
const MapDefinition* MapDefinition::GetDefinitionByName(const std::string& definitionName)
{
	// Safety check - definition exists
	bool definitionExists = (s_definitions.find(definitionName) != s_definitions.end());
	GUARANTEE_OR_DIE(definitionExists, Stringf("Error: Map definition \"%s\" doesn't exist.", definitionName.c_str()));

	return s_definitions[definitionName];
}


//-----------------------------------------------------------------------------------------------
// Parses the given XML element for the given attribute and interprets it as the name of a TileDefinition,
// returning the corresponding TileDefinition* from the static array
//
const MapDefinition* MapDefinition::ParseXMLAttribute(const XMLElement& mapDefinitionElement, const char* attributeName, MapDefinition* defaultValue)
{
	std::string defaultMapName = ParseXmlAttribute(mapDefinitionElement, attributeName, nullptr);

	if (defaultMapName.compare(std::string("")) == 0)
	{
		return defaultValue;
	}

	// Safety check - ensure the definition exists
	bool definitionExists = (s_definitions.find(defaultMapName) != s_definitions.end());
	GUARANTEE_OR_DIE(definitionExists, Stringf("Error: MapDefinition::ParseXMLAttribute does not have MapDefinition \"%s\".", defaultMapName.c_str()));

	return s_definitions[defaultMapName];
}

