/************************************************************************/
/* File: WaveManager.hpp
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
class EntityDefinition;

class WaveManager
{
public:
	//-----Public Methods-----
	
	WaveManager();
	~WaveManager();

	void	Initialize(const char* filename);
	void	CleanUp();

	void	Update();

	// Accessors
	int		GetWaveCount() const;
	bool	IsCurrentWaveFinal() const;
	int		GetCurrentWaveNumber() const;
	bool	IsCurrentWaveFinished() const;

	// Mutators
	void	StartNextWave();


public:
	//-----Public Data-----
	

private:
	//-----Private Methods-----
	
	void	InitializeCoreInfo(const XMLElement& rootElement);
	void	InitializeSpawnPoints(const XMLElement& rootElement);
	void	InitializeWaves(const XMLElement& rootElement);

	bool	PerformWaveEndCheck();

	int		GetTotalSpawnCount() const;
	int		GetSpawnCountForType(const EntityDefinition* definition) const;


private:
	//-----Private Data-----

	// State
	Stopwatch					m_spawnTick;

	bool						m_currWaveFinished = false;
	int							m_currWaveIndex = -1;
	int							m_totalSpawnedThisWave = 0;

	// Data
	std::vector<Wave*>			m_waves;
	std::vector<SpawnPoint*>	m_spawnPoints;
	int							m_maxSpawnedEntities = 100000;

};
