/************************************************************************/
/* File: MapGenStep_Mutate.cpp
/* Author: Andrew Chase
/* Date: November 18th, 2017
/* Bugs: None
/* Description: Implementation of the MapGenStep_Mutate (sub)class
/************************************************************************/
#include "Game/MapGenStep_Mutate.hpp"
#include "Game/TileDefinition.hpp"
#include "Game/Map.hpp"
#include "Engine/Core/XmlUtilities.hpp"
#include "Engine/Math/MathUtils.hpp"


//-----------------------------------------------------------------------------------------------
// Creates a Sprinkle generator step given the appropriate XMLElement
//
MapGenStep_Mutate::MapGenStep_Mutate(const tinyxml2::XMLElement& genStepXmlElement)
	: MapGenStep(genStepXmlElement)
{
	m_ifType	= TileDefinition::ParseXMLAttribute(genStepXmlElement, "ifType", m_ifType);
	m_toType	= TileDefinition::ParseXMLAttribute(genStepXmlElement, "toType", m_toType);
	m_mutateChance	= ParseXmlAttribute(genStepXmlElement, "chanceToMutate", m_mutateChance);
}



//-----------------------------------------------------------------------------------------------
// Destructor - Unused, but present for overriding
//
MapGenStep_Mutate::~MapGenStep_Mutate()
{
}


//-----------------------------------------------------------------------------------------------
// Applies the sprinkle effect taking into consideration the chance to run and number of iterations
//
void MapGenStep_Mutate::Run(Map& mapToAugment)
{
	bool shouldRunStep = CheckRandomChance(m_chanceToRun);

	if (shouldRunStep)
	{
		int numIterations = m_iterationsRange.GetRandomInRange();

		for (int i = 0; i < numIterations; i++)
		{
			ApplyMutate(mapToAugment);
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Searches the map for tiles of ifType (or all tiles if ifType not specified) and checks whether
// to change their type to toType
//
void MapGenStep_Mutate::ApplyMutate(Map& mapToAugment)
{
	IntVector2 mapDimensions = mapToAugment.GetDimensions();

	for (int rowIndex = 0; rowIndex < mapDimensions.y; ++rowIndex)
	{
		for (int colIndex = 0; colIndex < mapDimensions.x; ++colIndex)
		{
			// Don't change edge tiles - for safety
			if (rowIndex == 0 || rowIndex == (mapDimensions.y - 1) || colIndex == 0 || colIndex == (mapDimensions.x - 1)) { continue; }
			
			Tile* currTile = mapToAugment.GetTileFromCoords(IntVector2(colIndex, rowIndex));

			// Don't change the tile if it isn't the ifType
			if (m_ifType != nullptr && currTile->GetDefinition() != m_ifType) { continue; }

			// Check the mutate chance
			if (CheckRandomChance(m_mutateChance))
			{
				currTile->SetDefinition(m_toType);
			}	
		}
	}
}

