/************************************************************************/
/* File: CampaignStage.hpp
/* Author: Andrew Chase
/* Date: October 20th 2018
/* Description: Class to represent a single stage in a campaign
/************************************************************************/
#pragma once
#include "Game/Entity/EntitySpawn.hpp"
#include "Engine/Core/Image.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Core/Utility/XmlUtilities.hpp"
#include <vector>

class MapDefinition;
class EntitySpawnEvent;
class EntityDefinition;


enum eTransitionEdge
{
	EDGE_NORTH,
	EDGE_SOUTH,
	EDGE_EAST,
	EDGE_WEST
};

class CampaignStageData
{
public:
	friend class World;
	friend class CampaignManager;

	//-----Public Methods-----
	
	CampaignStageData() {}
	CampaignStageData(const XMLElement& element);
	~CampaignStageData();

	// Creates a copy of all spawn event prototypes for use in the manager
	void CloneAllEventPrototypes(CampaignManager* manager, std::vector<EntitySpawnEvent*>& out_spawnEventClones) const;
	
	void AddInitialSpawn(const EntityDefinition* definition, const Vector3& position, float orientation);


private:
	//-----Private Data-----
	
	const MapDefinition*					m_mapDefinition = nullptr;
	eTransitionEdge							m_edgeToEnter;

	std::vector<InitialStageSpawn_t>		m_initialStatics;
	std::vector<const EntitySpawnEvent*>	m_eventPrototypes;
	
};
