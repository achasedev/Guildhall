/************************************************************************/
/* File: CampaignStage.cpp
/* Author: Andrew Chase
/* Date: October 20th 2018
/* Description: Implementation of the CampaignStage class
/************************************************************************/
#include "Game/Framework/CampaignStage.hpp"
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


//---C FUNCTION----------------------------------------------------------------------------------
// Returns the correct subclass of spawn event to create based on the type
//

//-----------------------------------------------------------------------------------------------
// Constructor
//
CampaignStage::CampaignStage(const XMLElement& element)
{
	// Spawn info
	const XMLElement* spawnEventElement = element.FirstChildElement();

	while (spawnEventElement != nullptr)
	{
		EntitySpawnEvent* spawnEvent = EntitySpawnEvent::CreateSpawnEventForElement(*spawnEventElement);

		m_eventPrototypes.push_back(spawnEvent);

		spawnEventElement = spawnEventElement->NextSiblingElement("SpawnEvent");
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
CampaignStage::~CampaignStage()
{
}


//-----------------------------------------------------------------------------------------------
// Clones all entity spawn events as part of this stage to be updated by the CampaignManager
//
void CampaignStage::CloneAllEventPrototypes(CampaignManager* manager, std::vector<EntitySpawnEvent*>& out_spawnEventClones) const
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
