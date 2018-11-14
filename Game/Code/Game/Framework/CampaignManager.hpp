/************************************************************************/
/* File: CampaignManager.hpp
/* Author: Andrew Chase
/* Date: October 20th 2018
/* Description: Class to control per-stage entity spawning
/************************************************************************/
#pragma once
#include "Engine/Core/Time/Stopwatch.hpp"
#include "Engine/Core/Utility/XmlUtilities.hpp"
#include <vector>

class CampaignStage;
class SpawnPoint;
class EntityDefinition;

class CampaignManager
{
public:
	//-----Public Methods-----
	
	CampaignManager();
	~CampaignManager();

	void	Initialize(const char* filename);
	void	CleanUp();

	void	Update();

	// Accessors
	int		GetStageCount() const;
	bool	IsCurrentStageFinal() const;
	int		GetCurrentStageNumber() const;
	bool	IsCurrentStageFinished() const;

	// Mutators
	void	StartNextStage();


public:
	//-----Public Data-----
	

private:
	//-----Private Methods-----
	
	void	InitializeSpawnPoints(const XMLElement& rootElement);
	void	InitializeStages(const XMLElement& rootElement);

	bool	PerformStageEndCheck();

	int		GetTotalSpawnCount() const;
	int		GetSpawnCountForType(const EntityDefinition* definition) const;


private:
	//-----Private Data-----

	// State
	Stopwatch					m_stageTimer;
	Stopwatch					m_spawnTick;

	bool						m_currStageFinished = false;
	int							m_currStageIndex = -1;
	int							m_totalSpawnedThisStage = 0;

	// Data
	std::vector<CampaignStage*>			m_stages;
	std::vector<SpawnPoint*>	m_spawnPoints;
	int							m_maxSpawnedEntities = 100000;

};
