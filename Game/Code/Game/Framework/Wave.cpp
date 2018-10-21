/************************************************************************/
/* File: Wave.cpp
/* Author: Andrew Chase
/* Date: October 20th 2018
/* Description: Implementation of the Wave class
/************************************************************************/
#include "Game/Framework/Wave.hpp"
#include "Game/Entity/EntityDefinition.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Utility/ErrorWarningAssert.hpp"


//-----------------------------------------------------------------------------------------------
// Constructor
//
Wave::Wave(const XMLElement& element)
{
	// Base info
	m_maxSpawned = ParseXmlAttribute(element, "max_spawned", m_maxSpawned);

	// Spawn info
	const XMLElement* spawnElement = element.FirstChildElement();

	while (spawnElement != nullptr)
	{
		EntitySpawnEvent_t info;

		info.maxLiveSpawned		= ParseXmlAttribute(*spawnElement,	"max_spawned",		info.maxLiveSpawned);
		info.minLiveSpawned		= ParseXmlAttribute(*spawnElement,	"min_spawned",		info.minLiveSpawned);
		info.spawnDelay			= ParseXmlAttribute(*spawnElement,	"spawn_delay",		info.spawnDelay);
		info.countToSpawn		= ParseXmlAttribute(*spawnElement,	"total_to_spawn",	info.countToSpawn);

		// Calculate the max threshold given the min and max live counts
		int range = info.maxLiveSpawned - info.minLiveSpawned;
		info.maxLiveThreshold = Ceiling((float)range * 0.8f) + info.minLiveSpawned;

		std::string typeName = ParseXmlAttribute(*spawnElement, "entity", "");
		GUARANTEE_OR_DIE(typeName != "", "Error: Missing entity type in wave definition");
		info.definition = EntityDefinition::GetDefinition(typeName);

		m_events.push_back(info);

		spawnElement = spawnElement->NextSiblingElement();
	}
}


//-----------------------------------------------------------------------------------------------
// Destructor
//
Wave::~Wave()
{
}
