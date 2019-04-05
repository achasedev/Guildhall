#include "Game/Entity/EntityDefinition.hpp"
#include "Game/Framework/MapDefinition.hpp"
#include "Game/Framework/CampaignDefinition.hpp"
#include "Engine/Math/MathUtils.hpp"
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

	int numVolumesAlongX = 4;
	int numVolumesAlongZ = 4;
	float paddingFromEdgeOfMap = 10.f;
	float stepPerX = (256.f - 2.f * paddingFromEdgeOfMap) / (float)numVolumesAlongX;
	float stepPerZ = (256.f - 2.f * paddingFromEdgeOfMap) / (float)numVolumesAlongZ;

	Vector2 offsetToGridCellCenter = Vector2(stepPerX * 0.5f, stepPerZ * 0.5f);
	Vector2 gridBottomLeft = Vector2(paddingFromEdgeOfMap); // Don't worry about Y, the map always snaps them to ground

	for (int volumeGridZ = 0; volumeGridZ < numVolumesAlongZ; ++volumeGridZ)
	{
		for (int volumeGridX = 0; volumeGridX < numVolumesAlongX; ++volumeGridX)
		{
			int selectVolumeIndex = volumeGridZ * numVolumesAlongX + volumeGridX;
			selectVolumeIndex = ClampInt(selectVolumeIndex, 0, (int)selectVolumeDefs.size() - 1);

			const EntityDefinition* currVolume = selectVolumeDefs[selectVolumeIndex];
			IntVector3 volumeDimensions = currVolume->GetDimensions();

			InitialStageSpawn_t spawn;
			spawn.definition = currVolume;
			spawn.orientation = 180.f; // Makes then face upright

			Vector2 gridCellBottomLeft = gridBottomLeft + Vector2((float)volumeGridX * stepPerX, (float)volumeGridZ * stepPerZ);
			Vector2 gridCellCenter = gridCellBottomLeft + offsetToGridCellCenter;
			Vector2 entityPosition = gridCellCenter - Vector2(volumeDimensions.x / 2, volumeDimensions.z / 2);
			spawn.position = entityPosition;

			selectStage->m_initialSpawns.push_back(spawn);
		}
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

	if (!IsStringNullOrEmpty(mapName))
	{
		const MapDefinition* characterSelectMap = MapDefinition::GetDefinitionByName(mapName);
		GUARANTEE_OR_DIE(characterSelectMap != nullptr, Stringf("Campaign file %s cannot find map \"%s\" for the character select", filePath.c_str(), mapName.c_str()));

		// Create the stages, starting with the character select
		CampaignStage* selectStage = CreateCharacterSelectStage(characterSelectMap);
		newDef->m_stages.push_back(selectStage);

		newDef->m_hasCharacterSelect = true;
	}

	// Continue with the rest if it is specified
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
// Returns the number of campaigns currently loaded from disk
//
int CampaignDefinition::GetCampaignCount()
{
	return (int)s_campaignDefinitions.size();
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

