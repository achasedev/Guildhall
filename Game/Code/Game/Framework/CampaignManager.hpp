/************************************************************************/
/* File: CampaignManager.hpp
/* Author: Andrew Chase
/* Date: October 20th 2018
/* Description: Class to control per-stage entity spawning
/************************************************************************/
#pragma once
#include "Engine/Core/Time/Clock.hpp"
#include "Engine/Core/Time/Stopwatch.hpp"
#include "Engine/Core/Utility/XmlUtilities.hpp"
#include <vector>

class CampaignStage;
class EntityDefinition;
class EntitySpawnEvent;
class CampaignDefinition;

class CampaignManager
{
public:
	//-----Public Methods-----
	
	CampaignManager();
	~CampaignManager();

	void	Initialize(const CampaignDefinition* definition);
	void	CleanUp();

	void	Update();

	// Accessors
	int							GetStageCount() const;
	bool						IsCurrentStageFinal() const;
	int							GetCurrentStageNumber() const;
	bool						IsCurrentStageFinished() const;
	const CampaignStage*		GetCurrentStage() const;
	const CampaignStage*		GetNextStage() const;
	int							GetEnemyCountLeftInStage() const;
	int							GetEntityCountSpawnedThisStageSoFar() const;
	float						GetTimeIntoStage() const;
	float						GetCurrentDifficultyScale() const;

	// Mutators
	void						StartNextStage();
	void						RescaleToNewDifficulty(float newDifficultyScale);

	
private:
	//-----Private Methods-----
	
	int							GetCurrentLiveEnemyCount() const;


private:
	//-----Private Data-----

	// State
	Clock							m_spawnClock;
	Stopwatch						m_stageTimer;
	Stopwatch						m_spawnTick;

	std::vector<EntitySpawnEvent*>	m_currentSpawnEvents;

	bool							m_currStageFinished = false;
	int								m_currStageIndex = 0;
	int								m_totalSpawnedThisStage = 0;

	float							m_currentDifficultyScale = 1.0f;

	// Data
	const CampaignDefinition*		m_campaignDefinition = nullptr;

};
