/************************************************************************/
/* File: Wave.hpp
/* Author: Andrew Chase
/* Date: October 20th 2018
/* Description: Class to represent a single Entity wave
/************************************************************************/
#pragma once
#include "Engine/Core/Utility/XmlUtilities.hpp"
#include <vector>

class EntityDefinition;

// Single description for a spawn event
struct EntitySpawnEvent_t
{
	const EntityDefinition* definition = nullptr; // What to spawn
	int countToSpawn = 0;		// Total amount to spawn in this event
	int minLiveSpawned = 0;		// Ensure this many are on screen at all points in time
	int maxLiveSpawned = 0;		// Ensure we never have more than this many spawned at once
	int spawnDelay = 0;			// How many entities should spawn in the wave before this event should start
	int maxLiveThreshold = 0;	// If the live number falls below this value, attempt to spawn more
};

class Wave
{
public:
	friend class WaveManager;

	//-----Public Methods-----
	
	Wave(const XMLElement& element);
	~Wave();
	
	
private:
	//-----Private Data-----
	
	unsigned int					m_maxSpawned = 10000;
	std::vector<EntitySpawnEvent_t> m_events;
	
};
