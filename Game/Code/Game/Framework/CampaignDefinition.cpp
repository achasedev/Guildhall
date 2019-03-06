#include "Game/Entity/EntityDefinition.hpp"
#include "Game/Framework/MapDefinition.hpp"
#include "Game/Framework/CampaignDefinition.hpp"
#include "Engine/Core/Utility/StringUtils.hpp"
#include "Engine/Core/Utility/XmlUtilities.hpp"
#include "Engine/Core/Utility/ErrorWarningAssert.hpp"


std::map<std::string, const CampaignDefinition*>  CampaignDefinition::s_campaignDefinitions;


//---C FUNCTION----------------------------------------------------------------------------------
// Creates a character select stage using the given map
// Places a character select volume for each player
//
CampaignStage* CreateCharacterSelectStage(const MapDefinition* mapDefinition)
{
	CampaignStage* selectStage = new CampaignStage();

	selectStage->m_mapDefinition = mapDefinition;
	
	std::vector<const EntityDefinition*> selectVolumeDefs;
	EntityDefinition::GetAllCharacterSelectVolumeDefinitions(selectVolumeDefs);

	for (int i = 0; i < (int) selectVolumeDefs.size(); ++i)
	{
		InitialStageSpawn_t spawn;
		spawn.definition = selectVolumeDefs[i];
		spawn.orientation = 180.f;
		spawn.position = Vector2(20.f + (float)i * 50.f);

		selectStage->m_initialSpawns.push_back(spawn);
	}

	// We don't care about enter edge for this stage, since we don't use it
	return selectStage;
}


//-----------------------------------------------------------------------------------------------
// Loads the campaign file specified by filePath
//
void CampaignDefinition::LoadCampaign(const std::string filePath)
{
	// Load the file
	XMLDocument document;
	XMLError error = document.LoadFile(filePath.c_str());
	GUARANTEE_OR_DIE(error == tinyxml2::XML_SUCCESS, Stringf("Couldn't load map file %s", filePath.c_str()));

	// Get the root information
	const XMLElement* rootElement = document.RootElement();
	GUARANTEE_OR_DIE(rootElement != nullptr, Stringf("No root element specified in map %s", filePath.c_str()));

	CampaignDefinition* newDef = new CampaignDefinition();

	// Name
	newDef->m_name = ParseXmlAttribute(*rootElement, "name", "");
	GUARANTEE_OR_DIE(!IsStringNullOrEmpty(newDef->m_name), Stringf("Campaign file %s has no campaign name specified", filePath.c_str()));

	// Music
	newDef->m_backgroundMusicTrack = ParseXmlAttribute(*rootElement, "music", "");
	GUARANTEE_OR_DIE(!IsStringNullOrEmpty(newDef->m_backgroundMusicTrack), Stringf("Campaign file %s has no music file specified", filePath.c_str()));

	// Character Select Map
	std::string mapName = ParseXmlAttribute(*rootElement, "character_select_map", "");
	GUARANTEE_OR_DIE(!IsStringNullOrEmpty(mapName), Stringf("Campaign file %s has no character select map specified", filePath.c_str()));

	const MapDefinition* characterSelectMap = MapDefinition::GetDefinitionByName(mapName);
	GUARANTEE_OR_DIE(characterSelectMap != nullptr, Stringf("Campaign file %s cannot find map \"%s\" for the character select", filePath.c_str(), mapName.c_str()));

	// Create the stages, starting with the character select
	CampaignStage* selectStage = CreateCharacterSelectStage(characterSelectMap);
	newDef->m_stages.push_back(selectStage);

	const XMLElement* stageElement = rootElement->FirstChildElement("Stage");
	GUARANTEE_OR_DIE(stageElement != nullptr, Stringf("No stage elements specified in map %s", filePath.c_str()));

	while (stageElement != nullptr)
	{
		newDef->m_stages.push_back(new CampaignStage(*stageElement));

		stageElement = stageElement->NextSiblingElement("Stage");
	}

	// Add it to the registry
	AddDefinitionToRegistry(newDef);
}


//-----------------------------------------------------------------------------------------------
// Returns the campaign definition specified by the name, nullptr if it doesn't exist
//
const CampaignDefinition* CampaignDefinition::GetDefinitionByName(const std::string& name)
{
	bool exists = s_campaignDefinitions.find(name) != s_campaignDefinitions.end();
	if (exists)
	{
		return s_campaignDefinitions.at(name);
	}

	return nullptr;
}


//-----------------------------------------------------------------------------------------------
// Adds the given campaign definition to the registry, checking for duplicate names
//
void CampaignDefinition::AddDefinitionToRegistry(const CampaignDefinition* definition)
{
	bool alreadyExists = s_campaignDefinitions.find(definition->m_name) != s_campaignDefinitions.end();
	GUARANTEE_OR_DIE(!alreadyExists, Stringf("Duplicate campaign definition exists for name \"%s\"", definition->m_name.c_str()));

	s_campaignDefinitions[definition->m_name] = definition;
}

