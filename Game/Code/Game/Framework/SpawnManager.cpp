/************************************************************************/
/* File: SpawnManager.cpp
/* Author: Andrew Chase
/* Date: October 20th 2018
/* Description: Implementation of the SpawnManager class
/************************************************************************/
#include "Game/Framework/Game.hpp"
#include "Game/Framework/Wave.hpp"
#include "Game/Framework/SpawnPoint.hpp"
#include "Game/Framework/SpawnManager.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Utility/StringUtils.hpp"
#include "Engine/Core/Utility/XmlUtilities.hpp"
#include "Engine/Core/Utility/ErrorWarningAssert.hpp"


//-----------------------------------------------------------------------------------------------
// Constructor
//
SpawnManager::SpawnManager(const char* filename)
	: m_spawnTick(Stopwatch(Game::GetGameClock()))
{
	// Load the file
	XMLDocument document;
	XMLError error = document.LoadFile(filename);
	GUARANTEE_OR_DIE(error == XMLError::XML_SUCCESS, Stringf("Error: Couldn't open SpawnManager file %s", filename));

	// Get the root element
	const XMLElement* rootElement = document.RootElement();
	GUARANTEE_OR_DIE(rootElement != nullptr, Stringf("Error: SpawnManager file %s has no root element", filename));

	// Core
	InitializeCoreInfo(*rootElement);

	// Spawnpoints
	InitializeSpawnPoints(*rootElement);

	// Waves
	InitializeWaves(*rootElement);

	m_spawnTick.SetInterval(1.0f);
}


//-----------------------------------------------------------------------------------------------
// Destructor
//
SpawnManager::~SpawnManager()
{
	for (int i = 0; i < (int)m_waves.size(); ++i)
	{
		delete m_waves[i];
	}

	m_waves.clear();

	for (int i = 0; i < (int)m_spawnPoints.size(); ++i)
	{
		delete m_spawnPoints[i];
	}

	m_spawnPoints.clear();
}


//-----------------------------------------------------------------------------------------------
// Update
//
void SpawnManager::Update()
{
	// Check for end of wave
	CheckForCurrWaveFinished();

	if (m_currWaveFinished)
	{
		return;
	}

	// Check for spawn tick
	if (m_spawnTick.DecrementByIntervalAll() == 0)
	{
		return;
	}

	// Check the spawn conditions for the current wave
	Wave* wave = m_waves[m_currWaveIndex];

	int numSpawns = (int) wave->m_events.size();
	for (int spawnIndex = 0; spawnIndex < numSpawns; ++spawnIndex)
	{
		EntitySpawnEvent_t& event = wave->m_events[spawnIndex];

		// We've spawned all of this event
		if (event.countToSpawn <= 0)
		{
			continue;
		}

		int spawnCount = GetSpawnCountForType(event.definition);

		// If the number on the field is already above threshold, continue
		if (spawnCount >= event.maxLiveThreshold)
		{
			continue;
		}


		// If this event should still be delayed, continue
		if (m_numberOfEntitiesSpawned < event.spawnDelay)
		{
			continue;
		}

		// If the number is less than the min amount, force a spawn to the min amount
		int amountToSpawn = 0;
		int lowerBound = spawnCount;

		if (spawnCount < event.minLiveSpawned)
		{
			amountToSpawn += event.minLiveSpawned - spawnCount;
			lowerBound = event.minLiveSpawned;
		}

		// Add a random amount on top of that for variance
		int range = (int)(event.maxLiveSpawned - lowerBound);

		int addition = GetRandomIntInRange(0, range);
		amountToSpawn += addition;

		// Ensure we don't go over the max to spawn
		if (amountToSpawn > event.countToSpawn)
		{
			amountToSpawn = event.countToSpawn;
		}

		// Update how many we still have to spawn
		event.countToSpawn -= amountToSpawn;

		int spawnerIndex = GetRandomIntLessThan((int) m_spawnPoints.size());

		SpawnPoint* point = m_spawnPoints[spawnerIndex];

		for (int i = 0; i < amountToSpawn; ++i)
		{
			point->SpawnEntity(event.definition);
			m_numberOfEntitiesSpawned++;
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Parses for any core information for the spawn manager
//
void SpawnManager::InitializeCoreInfo(const XMLElement& rootElement)
{
	const XMLElement* coreElement = rootElement.FirstChildElement("Core");

	if (coreElement != nullptr)
	{
		m_maxSpawnedEntities = ParseXmlAttribute(*coreElement, "max_spawned_entities", m_maxSpawnedEntities);
	}
}


//-----------------------------------------------------------------------------------------------
// Parses for the spawn point information
//
void SpawnManager::InitializeSpawnPoints(const XMLElement& rootElement)
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
// Parses for the waves descriptions
//
void SpawnManager::InitializeWaves(const XMLElement& rootElement)
{
	const XMLElement* wavesElement = rootElement.FirstChildElement("Waves");

	GUARANTEE_OR_DIE(wavesElement != nullptr, "Error: SpawnManager file has no waves element");

	const XMLElement* currWaveElement = wavesElement->FirstChildElement();

	GUARANTEE_OR_DIE(currWaveElement != nullptr, "Error: SpawnManager file has no waves specified");

	while (currWaveElement != nullptr)
	{
		Wave* wave = new Wave(*currWaveElement);
		m_waves.push_back(wave);

		currWaveElement = currWaveElement->NextSiblingElement();
	}
}


//-----------------------------------------------------------------------------------------------
// Checks if all entities for this wave have spawned and are dead, signalling this wave is finished
//
bool SpawnManager::CheckForCurrWaveFinished()
{
	if (m_currWaveFinished)
	{
		return true;
	}

	Wave* wave = m_waves[m_currWaveIndex];

	bool allSpawnsDone = true;
	int numSpawns = (int) wave->m_events.size();
	for (int spawnIndex = 0; spawnIndex < numSpawns; ++spawnIndex)
	{
		if (wave->m_events[spawnIndex].countToSpawn > 0)
		{
			allSpawnsDone = false;
		}
	}

	m_currWaveFinished = allSpawnsDone && (GetTotalSpawnCount() == 0);

	return m_currWaveFinished;
}


//-----------------------------------------------------------------------------------------------
// Returns the total number of entities spawned by this manager
//
int SpawnManager::GetTotalSpawnCount() const
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
int SpawnManager::GetSpawnCountForType(const EntityDefinition* definition) const
{
	int numSpawners = (int)m_spawnPoints.size();

	int total = 0;
	for (int i = 0; i < numSpawners; ++i)
	{
		total += m_spawnPoints[i]->GetLiveSpawnCountForType(definition);
	}

	return total;
}
