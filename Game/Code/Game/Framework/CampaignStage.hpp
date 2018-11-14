/************************************************************************/
/* File: CampaignStage.hpp
/* Author: Andrew Chase
/* Date: October 20th 2018
/* Description: Class to represent a single stage in a campaign
/************************************************************************/
#pragma once
#include "Engine/Core/Utility/XmlUtilities.hpp"
#include <vector>

class EntityDefinition;

// Single description for a spawn event
struct EntitySpawnEvent_t
{
	const EntityDefinition* definition = nullptr; // What to spawn
	int		countToSpawn = 0;			// Total amount to spawn in this event
	int		spawnRate = 1;				// The number of entities to spawn per spawn tick
	int		spawnCountDelay = 0;		// How many entities should spawn in the stage before this event should start
	float	spawnTimeDelay = 0.f;		// How long to wait in time before this event should start
	int		spawnPointID = 0;			// Which spawn point to spawn at
};

class CampaignStage
{
public:
	friend class CampaignManager;

	//-----Public Methods-----
	
	CampaignStage(const XMLElement& element);
	~CampaignStage();
	
	
private:
	//-----Private Data-----
	
	std::vector<EntitySpawnEvent_t> m_events;
	
};
