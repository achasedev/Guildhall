/************************************************************************/
/* File: MapGenStep_FillAndEdge.cpp
/* Author: Andrew Chase
/* Date: November 2nd, 2017
/* Bugs: None
/* Description: Implementation of the MapGenStep_FillAndEdge (sub)class
/************************************************************************/
#include "Game/MapGenStep_FillAndEdge.hpp"
#include "Game/TileDefinition.hpp"
#include "Game/Map.hpp"
#include "Engine/Core/XmlUtilities.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"


//-----------------------------------------------------------------------------------------------
// Constructor - parses a FillAndEdge generation step element for the data and initializes accordingly
//
MapGenStep_FillAndEdge::MapGenStep_FillAndEdge(const tinyxml2::XMLElement& genStepXmlElement)
	: MapGenStep(genStepXmlElement)
{
	m_fillTileDef	= TileDefinition::ParseXMLAttribute(genStepXmlElement, "fillType", m_fillTileDef);
	GUARANTEE_OR_DIE(m_fillTileDef != nullptr, Stringf("Error: MapGenStep_FillAndEdge has no fill type specified"));

	m_edgeTileDef	= TileDefinition::ParseXMLAttribute(genStepXmlElement, "edgeType", m_edgeTileDef);
	GUARANTEE_OR_DIE(m_fillTileDef != nullptr, Stringf("Error: MapGenStep_FillAndEdge has no edge type specified"));

	m_edgeThickness = ParseXmlAttribute(genStepXmlElement, "edgeThickness", m_edgeThickness);
}


//-----------------------------------------------------------------------------------------------
// Destructor - not used
//
MapGenStep_FillAndEdge::~MapGenStep_FillAndEdge()
{

}


//-----------------------------------------------------------------------------------------------
// Overridden function that applies this generation step logic to the given map
//
void MapGenStep_FillAndEdge::Run(Map& mapToAugment)
{
	bool shouldRunStep = CheckRandomChance(m_chanceToRun);

	if (shouldRunStep)
	{
		int numIterations = m_iterationsRange.GetRandomInRange();

		for (int i = 0; i < numIterations; i++)
		{
			ApplyFillAndEdge(mapToAugment);
		}
	}
}


//-----------------------------------------------------------------------------------------------
// For FillAndEdge, this function fills the entire map with fillTileDef tiles and then lines the
// edge with edgeTileDef tiles
//
void MapGenStep_FillAndEdge::ApplyFillAndEdge(Map& mapToAugment)
{
	IntVector2 dimensions = mapToAugment.GetDimensions();

	for (int rowIndex = 0; rowIndex < dimensions.y; ++rowIndex)
	{
		for (int colIndex = 0; colIndex < dimensions.x; ++colIndex)
		{
			Tile* currTile = mapToAugment.GetTileFromCoords(IntVector2(colIndex, rowIndex));

			if (rowIndex < m_edgeThickness || rowIndex >= (dimensions.y - m_edgeThickness) || colIndex < m_edgeThickness || colIndex >= (dimensions.x - m_edgeThickness))
			{
				currTile->SetDefinition(m_edgeTileDef);
			}
			else
			{
				currTile->SetDefinition(m_fillTileDef);
			}
		}
	}
}
