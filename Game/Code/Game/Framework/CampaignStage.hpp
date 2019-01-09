/************************************************************************/
/* File: CampaignStage.hpp
/* Author: Andrew Chase
/* Date: October 20th 2018
/* Description: Class to represent a single stage in a campaign
/************************************************************************/
#pragma once
#include "Engine/Core/Image.hpp"
#include "Engine/Math/Vector3.hpp"
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

struct InitialStaticSpawn_t
{
	const EntityDefinition* definition = nullptr;
	Vector3 position;
	float orientation;
};


enum eTransitionEdge
{
	EDGE_NORTH,
	EDGE_SOUTH,
	EDGE_EAST,
	EDGE_WEST
};

class CampaignStage
{
public:
	friend class World;
	friend class CampaignManager;

	//-----Public Methods-----
	
	CampaignStage() {}
	CampaignStage(const XMLElement& element);
	~CampaignStage();

	void AddStaticSpawn(const EntityDefinition* definition, const Vector3& position, float orientation);
	
	
private:
	//-----Private Data-----
	
	std::string						m_mapName;
	eTransitionEdge					m_edgeToEnter;

	std::vector<InitialStaticSpawn_t> m_initialStatics;
	std::vector<EntitySpawnEvent_t> m_events;
	
};
