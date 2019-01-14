/************************************************************************/
/* File: CampaignManager.cpp
/* Author: Andrew Chase
/* Date: October 20th 2018
/* Description: Implementation of the CampaignManager class
/************************************************************************/
#include "Game/Framework/Game.hpp"
#include "Game/Framework/World.hpp"
#include "Game/Framework/SpawnPoint.hpp"
#include "Game/Entity/EntityDefinition.hpp"
#include "Game/Framework/CampaignStage.hpp"
#include "Game/Framework/CampaignManager.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Utility/StringUtils.hpp"
#include "Engine/Core/Utility/XmlUtilities.hpp"
#include "Engine/Core/Utility/ErrorWarningAssert.hpp"


//-----------------------------------------------------------------------------------------------
// Constructor
//
CampaignManager::CampaignManager()
	: m_stageTimer(Stopwatch(Game::GetGameClock()))
	, m_spawnTick(Stopwatch(Game::GetGameClock()))
{
}


//-----------------------------------------------------------------------------------------------
// Destructor
//
CampaignManager::~CampaignManager()
{
	CleanUp();

	delete m_characterSelectStage;
	m_characterSelectStage = nullptr;
}


//-----------------------------------------------------------------------------------------------
// Sets the manager's campaign data to the data given by the xml filename
//
void CampaignManager::Initialize(const char* filename)
{
	CleanUp();

	// Load the file
	XMLDocument document;
	XMLError error = document.LoadFile(filename);
	GUARANTEE_OR_DIE(error == XMLError::XML_SUCCESS, Stringf("Error: Couldn't open SpawnManager file %s", filename));

	// Get the root element
	const XMLElement* rootElement = document.RootElement();
	GUARANTEE_OR_DIE(rootElement != nullptr, Stringf("Error: SpawnManager file %s has no root element", filename));

	// Spawnpoints
	InitializeSpawnPoints(*rootElement);

	// Stages
	InitializeStages(*rootElement);

	m_spawnTick.SetInterval(1.0f);

	// Reset the state
	m_currStageFinished = false;
	m_currStageIndex = -1;
	m_totalSpawnedThisStage = 0;

	Game::ResetScore();

	// Start the music
	std::string musicName = ParseXmlAttribute(*rootElement, "music", "");

	if (!IsStringNullOrEmpty(musicName))
	{
		Game::PlayBGM(musicName);
	}
}


//-----------------------------------------------------------------------------------------------
// Resets the manager to be used again in another game
//
void CampaignManager::CleanUp()
{
	// Basic state
	m_spawnTick.Reset();
	m_currStageFinished = false;
	m_currStageIndex = -1;
	m_maxSpawnedEntities = 100000;
	m_totalSpawnedThisStage = 0;

	// Clean up stages, skipping the character select stage
	for (int i = 1; i < (int)m_stages.size(); ++i)
	{
		delete m_stages[i];
	}

	m_stages.clear();

	// Clean up spawn points
	for (int i = 0; i < (int)m_spawnPoints.size(); ++i)
	{
		delete m_spawnPoints[i];
	}

	m_spawnPoints.clear();
}


//-----------------------------------------------------------------------------------------------
// Update
//
void CampaignManager::Update()
{
	// Check for end of stage
	PerformStageEndCheck();

	if (m_currStageFinished)
	{
		return;
	}

	// Check for spawn tick
	if (m_spawnTick.DecrementByIntervalAll() == 0)
	{
		return;
	}

	// Check the spawn conditions for the current stage
	CampaignStage* stage = m_stages[m_currStageIndex];

	int numSpawns = (int) stage->m_events.size();
	for (int spawnIndex = 0; spawnIndex < numSpawns; ++spawnIndex)
	{
		EntitySpawnEvent_t& event = stage->m_events[spawnIndex];

		// We've spawned all of this event
		if (event.countToSpawn <= 0)
		{
			continue;
		}

		// If this event should still be delayed, continue
		if (m_totalSpawnedThisStage < event.spawnCountDelay || m_stageTimer.GetElapsedTime() < event.spawnTimeDelay)
		{
			continue;
		}

		// Spawn based on the spawn rate and the amount remaining to spawn
		int amountToSpawn = MinInt(event.countToSpawn, event.spawnRate);

		// Update how many we still have to spawn
		event.countToSpawn -= amountToSpawn;

		SpawnPoint* point = m_spawnPoints[event.spawnPointID];

		for (int i = 0; i < amountToSpawn; ++i)
		{
			point->SpawnEntity(event.definition);
			m_totalSpawnedThisStage++;
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Sets up the manager to begin the next stage
//
void CampaignManager::StartNextStage()
{
	m_currStageIndex++;
	m_currStageFinished = false;
	m_totalSpawnedThisStage = 0;

	m_stageTimer.Reset();
}


//-----------------------------------------------------------------------------------------------
// Returns whether the current stage has finished playing
//
bool CampaignManager::IsCurrentStageFinished() const
{
	return m_currStageFinished;
}


//-----------------------------------------------------------------------------------------------
// Returns the next stage of the manager, for transitions
//
CampaignStage* CampaignManager::GetNextStage() const
{
	if (IsCurrentStageFinal())
	{
		return nullptr;
	}

	return m_stages[m_currStageIndex + 1];
}


//-----------------------------------------------------------------------------------------------
// Returns whether the current stage is the last stage, for testing victory
//
bool CampaignManager::IsCurrentStageFinal() const
{
	return (m_currStageIndex == (int)m_stages.size() - 1);
}


//-----------------------------------------------------------------------------------------------
// Parses for the spawn point information
//
void CampaignManager::InitializeSpawnPoints(const XMLElement& rootElement)
{
	const XMLElement* pointsElement = rootElement.FirstChildElement("SpawnPoints");

	GUARANTEE_OR_DIE(pointsElement != nullptr, "Error: SpawnManager file has no SpawnPoints element");

	const XMLElement* currPointElement = pointsElement->FirstChildElement();

	GUARANTEE_OR_DIE(currPointElement != nullptr, "Error: SpawnManager file has no spawn points specified");

	while (currPointElement != nullptr)
	{
		SpawnPoint* point = new SpawnPoint(*currPointElement);
		m_spawnPoints.push_back(point);

		currPointElement = currPointElement->NextSiblingElement();
	}
}


//-----------------------------------------------------------------------------------------------
// Parses for the stages descriptions
//
void CampaignManager::InitializeStages(const XMLElement& rootElement)
{
	// Push in the character select stage to be able to transition into it
	if (m_characterSelectStage == nullptr)
	{
		XMLDocument document;
		document.LoadFile("Data/CharacterSelect.stage");

		const XMLElement* root = document.RootElement();
		m_characterSelectStage = new CampaignStage();
		m_characterSelectStage->m_mapName = ParseXmlAttribute(*root, "map", "CharacterSelect");

		const XMLElement* entityElement = root->FirstChildElement();
		GUARANTEE_OR_DIE(entityElement != nullptr, "Error: Character Select has no characters!");

		while (entityElement != nullptr)
		{
			// Get the character select definition
			std::string playerCharacterName = ParseXmlAttribute(*entityElement, "character", "");
			const EntityDefinition* characterDefinition = EntityDefinition::GetDefinition(playerCharacterName);

			GUARANTEE_OR_DIE(characterDefinition != nullptr, Stringf("Error: Character \"%s\" doesn't exist!", playerCharacterName.c_str()));

			// Try to get the character select volume for this player
			std::string volumeName = playerCharacterName + "_SelectVolume";

			const EntityDefinition* volumeDefinition = EntityDefinition::GetDefinition(volumeName);

			if (volumeDefinition == nullptr)
			{
				// No volume created yet for this character, so create one now (prevents needed to define one in data)
				EntityDefinition* volumeDefinitionTemp = new EntityDefinition();
				volumeDefinitionTemp->m_name = volumeName;
				volumeDefinitionTemp->m_collisionDef.layer = COLLISION_LAYER_WORLD;
				volumeDefinitionTemp->m_collisionDef.m_response = COLLISION_RESPONSE_IGNORE_CORRECTION;
				volumeDefinitionTemp->m_physicsType = PHYSICS_TYPE_STATIC;
				volumeDefinitionTemp->m_playerCharacterDefinition = characterDefinition;
				volumeDefinitionTemp->m_defaultSprite = characterDefinition->m_defaultSprite;

				EntityDefinition::AddDefinition(volumeDefinitionTemp);
				volumeDefinition = volumeDefinitionTemp;
			}

			// Parse the position and orientation
			Vector3 position = ParseXmlAttribute(*entityElement, "position", Vector3::ZERO);
			float orientation = ParseXmlAttribute(*entityElement, "orientation", 0.f);

			// Add in the static to be spawned when the world is spawned
			m_characterSelectStage->AddStaticSpawn(volumeDefinition, position, orientation);

			// Move to next element
			entityElement = entityElement->NextSiblingElement();
		}
	}
	
	// Put the character select stage first so we start with it
	m_stages.push_back(m_characterSelectStage);

	// Initialize all the stages in the campaign
	const XMLElement* stagesElement = rootElement.FirstChildElement("Stages");
	GUARANTEE_OR_DIE(stagesElement != nullptr, "Error: SpawnManager file has no stages element");

	const XMLElement* currStageElement = stagesElement->FirstChildElement();
	GUARANTEE_OR_DIE(currStageElement != nullptr, "Error: SpawnManager file has no stages specified");

	while (currStageElement != nullptr)
	{
		CampaignStage* stage = new CampaignStage(*currStageElement);
		m_stages.push_back(stage);

		currStageElement = currStageElement->NextSiblingElement();
	}
}


//-----------------------------------------------------------------------------------------------
// Checks if all entities for this stage have spawned and are dead, signalling this stage is finished
//
bool CampaignManager::PerformStageEndCheck()
{
	if (m_currStageFinished)
	{
		return true;
	}

	CampaignStage* stage = m_stages[m_currStageIndex];

	bool allSpawnsDone = true;
	int numSpawns = (int) stage->m_events.size();
	for (int spawnIndex = 0; spawnIndex < numSpawns; ++spawnIndex)
	{
		if (stage->m_events[spawnIndex].countToSpawn > 0)
		{
			allSpawnsDone = false;
		}
	}

	m_currStageFinished = allSpawnsDone && (GetTotalSpawnCount() == 0);

	return m_currStageFinished;
}


//-----------------------------------------------------------------------------------------------
// Returns the total number of entities spawned by this manager
//
int CampaignManager::GetTotalSpawnCount() const
{
	int numSpawners = (int) m_spawnPoints.size();

	int total = 0;
	for (int i = 0; i < numSpawners; ++i)
	{
		total += m_spawnPoints[i]->GetLiveSpawnCount();
	}

	return total;
}


//-----------------------------------------------------------------------------------------------
// Returns the number of entities of the given definition are spawned currently
//
int CampaignManager::GetSpawnCountForType(const EntityDefinition* definition) const
{
	int numSpawners = (int)m_spawnPoints.size();

	int total = 0;
	for (int i = 0; i < numSpawners; ++i)
	{
		total += m_spawnPoints[i]->GetLiveSpawnCountForType(definition);
	}

	return total;
}


//-----------------------------------------------------------------------------------------------
// Returns the current stage index
//
int CampaignManager::GetCurrentStageNumber() const
{
	return m_currStageIndex;
}


//-----------------------------------------------------------------------------------------------
// Returns the number of stages currently in the campaign manager
//
int CampaignManager::GetStageCount() const
{
	return (int)m_stages.size();
}
