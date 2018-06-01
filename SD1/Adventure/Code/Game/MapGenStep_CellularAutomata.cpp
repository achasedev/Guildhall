/************************************************************************/
/* File: MapGenStep_CellularAutomata.hpp
/* Author: Andrew Chase
/* Date: November 5th, 2017
/* Bugs: None
/* Description: Implementation of the CellularAutomata MapGenStep
/************************************************************************/
#include <vector>
#include "Game/MapGenStep_CellularAutomata.hpp"
#include "Game/TileDefinition.hpp"
#include "Game/Map.hpp"
#include "Engine/Core/XmlUtilities.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"


//-----------------------------------------------------------------------------------------------
// Constructor - constructs from a MapGenStep XMLElement
//
MapGenStep_CellularAutomata::MapGenStep_CellularAutomata(const tinyxml2::XMLElement& genStepXmlElement)
	: MapGenStep(genStepXmlElement)
{
	m_ifType			= TileDefinition::ParseXMLAttribute(genStepXmlElement, "ifType", m_ifType);	// nullptr on ifType means change any tile
	m_ifNeighborType	= TileDefinition::ParseXMLAttribute(genStepXmlElement, "toType", m_ifNeighborType); // nullptr on ifNeighborType means any tile
	m_toType			= TileDefinition::ParseXMLAttribute(genStepXmlElement, "toType", m_toType);
	GUARANTEE_OR_DIE(m_toType != nullptr, Stringf("Error: CellularAutomata constructor has no toType specified."));

	m_ifNeighborCount	= ParseXmlAttribute(genStepXmlElement, "ifNeighborCount", m_ifNeighborCount);
	m_chanceToMutate	= ParseXmlAttribute(genStepXmlElement, "chanceToMutate", m_chanceToMutate);
}


//-----------------------------------------------------------------------------------------------
// Destructor - unused
//
MapGenStep_CellularAutomata::~MapGenStep_CellularAutomata()
{
}


//-----------------------------------------------------------------------------------------------
// Runs the CellularAutomata steps given the number of iterations and chance to run
//
void MapGenStep_CellularAutomata::Run(Map& mapToAugment)
{
	bool shouldRunStep = CheckRandomChance(m_chanceToRun);

	if (shouldRunStep)
	{
		int numIterations = m_iterationsRange.GetRandomInRange();

		for (int i = 0; i < numIterations; i++)
		{
			ApplyCellularAutomata(mapToAugment);
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Applies the CellularAutomata step
//
void MapGenStep_CellularAutomata::ApplyCellularAutomata(Map& mapToAugment)
{
	std::vector<IntVector2> tileCoordsToChange;
	IntVector2 mapDimensions = mapToAugment.GetDimensions();
	int numTiles = mapDimensions.x * mapDimensions.y;

	for (int tileIndex = 0; tileIndex < numTiles; tileIndex++)
	{
		Tile* currTile = mapToAugment.GetTileFromIndex(tileIndex);
		
		// If the tile is invalid, i.e. off edge of map, continue
		if (currTile == nullptr) { continue; }

		IntVector2 currCoords = currTile->GetTileCoords();
		
		// If the type is not the ifType continue
		if ((currTile->GetDefinition() != m_ifType) && (m_ifType != nullptr)) { continue; }

		// Count neighbors of type to see if we should apply the change
		int numNeighborsOfType = CountNeighborsOfNeighborType(currCoords, mapToAugment);

		// Change the tile if there are enough neighbors and if we pass mutate chance
		if (m_ifNeighborCount.IsValueInRange(numNeighborsOfType) && CheckRandomChance(m_chanceToMutate))
		{
			tileCoordsToChange.push_back(currCoords);
		}
	}

	// Now iterate over the coordinates and set definitions
	for (int changeIndex = 0; changeIndex < static_cast<int>(tileCoordsToChange.size()); changeIndex++)
	{
		IntVector2 changeCoords = tileCoordsToChange[changeIndex];

		Tile* changeTile = mapToAugment.GetTileFromCoords(changeCoords);
		changeTile->SetDefinition(m_toType);
	}
}


//-----------------------------------------------------------------------------------------------
// Counts the number of neighbors to currCoords that are of m_ifNeighborType and returns the count
//
int MapGenStep_CellularAutomata::CountNeighborsOfNeighborType(const IntVector2& currCoords, Map& mapToAugment) const
{
	int numNeighborsOfType = 0;

	Tile* northTile			= mapToAugment.GetTileFromCoords(currCoords + IntVector2::STEP_NORTH);
	Tile* southTile			= mapToAugment.GetTileFromCoords(currCoords + IntVector2::STEP_SOUTH);
	Tile* eastTile			= mapToAugment.GetTileFromCoords(currCoords + IntVector2::STEP_EAST);
	Tile* westTile			= mapToAugment.GetTileFromCoords(currCoords + IntVector2::STEP_WEST);
	Tile* northEastTile		= mapToAugment.GetTileFromCoords(currCoords + IntVector2::STEP_NORTHEAST);
	Tile* northWestTile		= mapToAugment.GetTileFromCoords(currCoords + IntVector2::STEP_NORTHWEST);
	Tile* southEastTile		= mapToAugment.GetTileFromCoords(currCoords + IntVector2::STEP_SOUTHEAST);
	Tile* southWestTile		= mapToAugment.GetTileFromCoords(currCoords + IntVector2::STEP_SOUTHWEST);


	if (northTile		!= nullptr && (northTile->GetDefinition() == m_ifNeighborType || m_ifNeighborType == nullptr))		{ numNeighborsOfType++; }	// North
	if (southTile		!= nullptr && (southTile->GetDefinition() == m_ifNeighborType || m_ifNeighborType == nullptr))		{ numNeighborsOfType++; }	// South
	if (eastTile		!= nullptr && (eastTile->GetDefinition() == m_ifNeighborType || m_ifNeighborType == nullptr))		{ numNeighborsOfType++; }	// East
	if (westTile		!= nullptr && (westTile->GetDefinition() == m_ifNeighborType || m_ifNeighborType == nullptr))		{ numNeighborsOfType++; }	// West

	if (northEastTile	!= nullptr && (northEastTile->GetDefinition() == m_ifNeighborType || m_ifNeighborType == nullptr))	{ numNeighborsOfType++; }	// NorthEast
	if (northWestTile	!= nullptr && (northWestTile->GetDefinition() == m_ifNeighborType || m_ifNeighborType == nullptr))	{ numNeighborsOfType++; }	// NorthWest
	if (southEastTile	!= nullptr && (southEastTile->GetDefinition() == m_ifNeighborType || m_ifNeighborType == nullptr))	{ numNeighborsOfType++; }	// SouthEast
	if (southWestTile	!= nullptr && (southWestTile->GetDefinition() == m_ifNeighborType || m_ifNeighborType == nullptr))	{ numNeighborsOfType++; }	// SouthWest

	return numNeighborsOfType;
}
