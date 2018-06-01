/************************************************************************/
/* File: MapDefinition.hpp
/* Author: Andrew Chase
/* Date: November 2nd, 2017
/* Bugs: None
/* Description: Class to represent a map type and its generation method
/************************************************************************/
#pragma once
#include <string>
#include <map>
#include <vector>
#include "Game/MapGenStep.hpp"
#include "Engine/Math/IntRange.hpp"
#include "ThirdParty/tinyxml2/tinyxml2.h"

class TileDefinition;

class MapDefinition
{
public:
	//-----Public Methods-----

	explicit MapDefinition(const tinyxml2::XMLElement& mapDefinitionElement);
	std::string GetName() const;
	const TileDefinition* GetDefaultTile() const;
	std::vector<MapGenStep*> GetGenerators() const;
	int GetRandomWidthInRange() const;
	int GetRandomHeightInRange() const;

	static void LoadDefinitions();
	static const MapDefinition* GetDefinitionByName(const std::string& definitionName);

	static const MapDefinition* ParseXMLAttribute(const XMLElement& mapDefinitionElement, const char* attributeName, MapDefinition* defaultValue);


private:
	//-----Private Data-----

	std::string							m_name = "";					// Name of the MapDefinition
	const TileDefinition*				m_defaultTile = nullptr;		// Default Tile to fill this map with
	IntRange							m_width = IntRange(0,0);		// Width range of this map type
	IntRange							m_height = IntRange(0,0);		// Height range of this map type
	std::vector<MapGenStep*>			m_generationSteps;				// List of steps used to make this map type


	static std::map<std::string, MapDefinition*>	s_definitions;	// Loads the definition data from file
};