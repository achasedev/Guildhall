/************************************************************************/
/* File: SpawnManager.hpp
/* Author: Andrew Chase
/* Date: October 20th 2018
/* Description: Class to control per-wave entity spawning
/************************************************************************/
#pragma once
#include "Engine/Core/Time/Stopwatch.hpp"
#include "Engine/Core/Utility/XmlUtilities.hpp"
#include <vector>

class Wave;
class SpawnPoint;

class SpawnManager
{
public:
	//-----Public Methods-----
	
	SpawnManager(const char* filename);
	~SpawnManager();

	void Update();

	void StartNextWave();
	bool IsCurrentWaveFinished() const;


public:
	//-----Public Data-----
	

private:
	//-----Private Methods-----
	
	void InitializeCoreInfo(const XMLElement& rootElement);
	void InitializeSpawnPoints(const XMLElement& rootElement);
	void InitializeWaves(const XMLElement& rootElement);

	bool PerformWaveEndCheck();

	int GetTotalSpawnCount() const;
	int GetSpawnCountForType(const EntityDefinition* definition) const;


private:
	//-----Private Data-----
	
	Stopwatch m_spawnTick;

	bool m_currWaveFinished = false;

	unsigned int m_currWaveIndex = 0;
	std::vector<Wave*> m_waves;
	std::vector<SpawnPoint*> m_spawnPoints;

	int m_maxSpawnedEntities = 100000;
	int m_numberOfEntitiesSpawned = 0;
};
