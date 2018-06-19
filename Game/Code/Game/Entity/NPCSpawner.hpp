/************************************************************************/
/* File: NPCSpawner.hpp
/* Author: Andrew Chase
/* Date: June 18th, 2018
/* Description: Class to represent a spawner for NPC controlled tanks
/************************************************************************/
#pragma once
#include "Engine/Core/GameObject.hpp"

class NPCSpawner : public GameObject
{
public:
	//-----Public Methods-----

	NPCSpawner(const Vector3& position, unsigned int teamIndex);
	~NPCSpawner();

	virtual void Update(float deltaTime) override;

	// Mutators
	void SetSpawnRatePerMinute(float spawnRate);
	

private:
	//-----Private Methods-----

	void SpawnEntity() const;


private:
	//-----Private Data-----

	Stopwatch*		m_stopwatch;
	unsigned int	m_teamIndex;
	float			m_timeBetweenSpawns;

	static constexpr float DEFAULT_TIME_BETWEEN_SPAWNS = 5.f; // 12 spawns a minute

};
