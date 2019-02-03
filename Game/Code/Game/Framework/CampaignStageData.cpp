/************************************************************************/
/* File: CampaignStage.cpp
/* Author: Andrew Chase
/* Date: October 20th 2018
/* Description: Implementation of the CampaignStage class
/************************************************************************/
#include "Game/Framework/CampaignStageData.hpp"
#include "Game/Framework/MapDefinition.hpp"
#include "Game/Entity/EntityDefinition.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Utility/ErrorWarningAssert.hpp"

//- C FUNCTION ----------------------------------------------------------------------------------
// Converts the string representation of the enter edge into the enum
//
eTransitionEdge GetEdgeForString(const std::string& text)
{
	if		(text == "north")	{ return EDGE_NORTH; }
	else if (text == "south")	{ return EDGE_SOUTH; }
	else if (text == "east")	{ return EDGE_EAST; }
	else if (text == "west")	{ return EDGE_WEST; }
	else
	{
		return EDGE_EAST;
	}
}


//-----------------------------------------------------------------------------------------------
// Constructor
//
CampaignStageData::CampaignStageData(const XMLElement& element)
{
	// Spawn info
	const XMLElement* spawnElement = element.FirstChildElement();

	while (spawnElement != nullptr)
	{
		EntitySpawnEvent_t info;

		info.spawnCountDelay	= ParseXmlAttribute(*spawnElement,	"spawn_count_delay",	info.spawnCountDelay);
		info.spawnTimeDelay		= ParseXmlAttribute(*spawnElement,	"spawn_time_delay",		info.spawnTimeDelay);
		info.totalToSpawn		= ParseXmlAttribute(*spawnElement,	"total_to_spawn",		info.totalToSpawn);
		info.spawnRate			= ParseXmlAttribute(*spawnElement,	"spawn_rate",			info.spawnRate);
		info.spawnPointID		= ParseXmlAttribute(*spawnElement,	"spawn_point",			info.spawnPointID);

		std::string typeName = ParseXmlAttribute(*spawnElement, "entity", "");
		GUARANTEE_OR_DIE(typeName != "", "Error: Missing entity type in stage definition");
		info.definition = EntityDefinition::GetDefinition(typeName);

		m_eventPrototypes.push_back(info);

		spawnElement = spawnElement->NextSiblingElement();
	}

	std::string mapName = ParseXmlAttribute(element, "map", "");
	GUARANTEE_OR_DIE(!IsStringNullOrEmpty(mapName), Stringf("Campaign stage defined with a missing map name"));

	m_mapDefinition = MapDefinition::GetDefinitionByName(mapName);
	GUARANTEE_OR_DIE(m_mapDefinition != nullptr, Stringf("Campaign stage is defined with map name \"%s\" that doesn't exist", mapName.c_str()));

	std::string edgeText = ParseXmlAttribute(element, "enter", "");
	m_edgeToEnter = GetEdgeForString(edgeText);
}


//-----------------------------------------------------------------------------------------------
// Destructor
//
CampaignStageData::~CampaignStageData()
{
}


//-----------------------------------------------------------------------------------------------
// Clones all entity spawn events as part of this stage to be updated by the CampaignManager
//
void CampaignStageData::CloneAllEventPrototypes(CampaignManager* manager, std::vector<EntitySpawnEvent*>& out_spawnEventClones) const
{
	// Safety check to ensure it's empty
	for (int i = 0; i < (int)out_spawnEventClones.size(); ++i)
	{
		delete out_spawnEventClones[i];
	}

	out_spawnEventClones.clear();

	int numEvents = (int) m_eventPrototypes.size();
	for (int eventIndex = 0; eventIndex < numEvents; ++eventIndex)
	{
		EntitySpawnEvent* clone = m_eventPrototypes[eventIndex]->Clone(manager);
		out_spawnEventClones.push_back(clone);
	}
}


//-----------------------------------------------------------------------------------------------
// Adds the static spawn to the stage
//
void CampaignStageData::AddInitialSpawn(const EntityDefinition* definition, const Vector3& position, float orientation)
{
	InitialStageSpawn_t spawn;
	spawn.definition = definition;
	spawn.position = position;
	spawn.orientation = orientation;

	m_initialStatics.push_back(spawn);
}
