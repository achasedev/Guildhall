/************************************************************************/
/* File: MapGenStep_SpawnActor.cpp
/* Author: Andrew Chase
/* Date: November 27th, 2017
/* Bugs: None
/* Description: Implementatino of the SpawnActor MapGenStep
/************************************************************************/
#include "Game/MapGenStep_SpawnActor.hpp"
#include "Game/ActorDefinition.hpp"
#include "Game/Map.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/XmlUtilities.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"


//-----------------------------------------------------------------------------------------------
// Constructor - constructs from an XMLElement
//
MapGenStep_SpawnActor::MapGenStep_SpawnActor(const XMLElement& genStepXmlElement)
	: MapGenStep(genStepXmlElement)
{
	m_actorName			= ParseXmlAttribute(genStepXmlElement, "name", m_actorName);
	m_tileToSpawnOn		= TileDefinition::ParseXMLAttribute(genStepXmlElement, "spawnTile", nullptr);
	m_actorDefinition	= ActorDefinition::ParseXMLAttribute(genStepXmlElement, "type", nullptr);
}


//-----------------------------------------------------------------------------------------------
// Destructor - unused
//
MapGenStep_SpawnActor::~MapGenStep_SpawnActor()
{
}


//-----------------------------------------------------------------------------------------------
// Spawns the actor taking into consideration the chance to run and number of iterations
//
void MapGenStep_SpawnActor::Run(Map& mapToAugment)
{
	bool shouldRunStep = CheckRandomChance(m_chanceToRun);

	if (shouldRunStep)
	{
		int numIterations = m_iterationsRange.GetRandomInRange();

		for (int i = 0; i < numIterations; i++)
		{
			SpawnActor(mapToAugment);
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Spawns the actor on the map, given the generation step details
//
void MapGenStep_SpawnActor::SpawnActor(Map& mapToAugment)
{
	Tile* randomTile = mapToAugment.GetRandomTileOfType(m_tileToSpawnOn);
	Vector2 spawnPosition = randomTile->GetCenterPosition();

	mapToAugment.SpawnActor(spawnPosition + Vector2::GetRandomVector(0.05f), 0.f, m_actorDefinition, m_actorName);
}
