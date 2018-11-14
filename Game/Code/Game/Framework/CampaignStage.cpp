/************************************************************************/
/* File: CampaignStage.cpp
/* Author: Andrew Chase
/* Date: October 20th 2018
/* Description: Implementation of the CampaignStage class
/************************************************************************/
#include "Game/Framework/CampaignStage.hpp"
#include "Game/Entity/EntityDefinition.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Utility/ErrorWarningAssert.hpp"


//-----------------------------------------------------------------------------------------------
// Constructor
//
CampaignStage::CampaignStage(const XMLElement& element)
{
	// Spawn info
	const XMLElement* spawnElement = element.FirstChildElement();

	while (spawnElement != nullptr)
	{
		EntitySpawnEvent_t info;

		info.spawnCountDelay	= ParseXmlAttribute(*spawnElement,	"spawn_count_delay",	info.spawnCountDelay);
		info.spawnTimeDelay		= ParseXmlAttribute(*spawnElement,	"spawn_time_delay",		info.spawnTimeDelay);
		info.countToSpawn		= ParseXmlAttribute(*spawnElement,	"total_to_spawn",		info.countToSpawn);
		info.spawnRate			= ParseXmlAttribute(*spawnElement,	"spawn_rate",			info.spawnRate);
		info.spawnPointID		= ParseXmlAttribute(*spawnElement,	"spawn_point",			info.spawnPointID);

		std::string typeName = ParseXmlAttribute(*spawnElement, "entity", "");
		GUARANTEE_OR_DIE(typeName != "", "Error: Missing entity type in stage definition");
		info.definition = EntityDefinition::GetDefinition(typeName);

		m_events.push_back(info);

		spawnElement = spawnElement->NextSiblingElement();
	}

	// Height map
	std::string heightMapPath = ParseXmlAttribute(element, "height_map", heightMapPath);
	m_heightMapImage.LoadFromFile(heightMapPath);

	m_maxTerrainHeight = ParseXmlAttribute(element, "max_terrain_height", m_maxTerrainHeight);

	// We don't support anything over 64
	m_maxTerrainHeight = MinInt(m_maxTerrainHeight, 64);
}


//-----------------------------------------------------------------------------------------------
// Destructor
//
CampaignStage::~CampaignStage()
{
}
