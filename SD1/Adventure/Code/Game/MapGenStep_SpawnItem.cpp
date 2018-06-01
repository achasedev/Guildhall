/************************************************************************/
/* File: MapGenStep_SpawnItem.cpp
/* Author: Andrew Chase
/* Date: November 27th, 2017
/* Bugs: None
/* Description: Implementation of the SpawnItem MapGenStep
/************************************************************************/
#include "Game/MapGenStep_SpawnItem.hpp"
#include "Game/ItemDefinition.hpp"
#include "Game/Map.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/XmlUtilities.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"


//-----------------------------------------------------------------------------------------------
// Constructor - constructs from an XMLElement
//
MapGenStep_SpawnItem::MapGenStep_SpawnItem(const XMLElement& genStepXmlElement)
	: MapGenStep(genStepXmlElement)
{
	m_itemName			= ParseXmlAttribute(genStepXmlElement, "name", m_itemName);
	m_tileToSpawnOn		= TileDefinition::ParseXMLAttribute(genStepXmlElement, "spawnTile", nullptr);
	m_itemDefinition	= ItemDefinition::ParseXMLAttribute(genStepXmlElement, "type", nullptr);
}


//-----------------------------------------------------------------------------------------------
// Destructor - unused
//
MapGenStep_SpawnItem::~MapGenStep_SpawnItem()
{
}


//-----------------------------------------------------------------------------------------------
// Spawns the item taking into consideration the chance to run and number of iterations
//
void MapGenStep_SpawnItem::Run(Map& mapToAugment)
{
	bool shouldRunStep = CheckRandomChance(m_chanceToRun);

	if (shouldRunStep)
	{
		int numIterations = m_iterationsRange.GetRandomInRange();

		for (int i = 0; i < numIterations; i++)
		{
			SpawnItem(mapToAugment);
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Spawns the item on the map, given the generation step details
//
void MapGenStep_SpawnItem::SpawnItem(Map& mapToAugment)
{
	Tile* randomTile = mapToAugment.GetRandomTileOfType(m_tileToSpawnOn);
	Vector2 spawnPosition = randomTile->GetCenterPosition();

	mapToAugment.SpawnItem(spawnPosition + Vector2::GetRandomVector(0.05f), 0.f, m_itemDefinition, m_itemName);
}
