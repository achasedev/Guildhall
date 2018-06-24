/************************************************************************/
/* File: NPCSpawner.hpp
/* Author: Andrew Chase
/* Date: June 18th, 2018
/* Description: Class to represent a spawner for NPC controlled tanks
/************************************************************************/
#pragma once
#include "Game/Entity/GameEntity.hpp"

class Stopwatch;

class Spawner : public GameEntity
{
public:
	//-----Public Methods-----

	Spawner(const Vector3& position, unsigned int teamIndex);
	~Spawner();

	virtual void Update(float deltaTime) override;

	// Mutators
	void SetSpawnRatePerMinute(float spawnRate);
	

private:
	//-----Private Methods-----

	virtual void SpawnEntity() const = 0;


private:
	//-----Private Data-----

	Stopwatch*		m_stopwatch;
	float			m_timeBetweenSpawns;

	static constexpr float DEFAULT_TIME_BETWEEN_SPAWNS = 5.f; // 12 spawns a minute

};
