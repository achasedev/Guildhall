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

// Single description for a spawn
struct EntitySpawnEvent_t
{
	const EntityDefinition* definition = nullptr;
	int countToSpawn = 0;
	int minLiveSpawned = 0;
	int maxLiveSpawned = 0;
	int spawnDelay = 0;
	int maxLiveThreshold = 0;
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
