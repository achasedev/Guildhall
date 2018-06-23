/************************************************************************/
/* File: NPCSpawner.cpp
/* Author: Andrew Chase
/* Date: June 18th, 2018
/* Description: Implementation of the NPCSpawner class
/************************************************************************/
#include "Game/Framework/Game.hpp"
#include "Game/Entity/NPCTank.hpp"
#include "Game/Environment/Map.hpp"
#include "Game/Entity/NPCSpawner.hpp"

#include "Engine/Assets/AssetDB.hpp"
#include "Engine/Core/Time/Stopwatch.hpp"
#include "Engine/Rendering/Core/RenderScene.hpp"
#include "Engine/Rendering/Materials/MaterialInstance.hpp"

#include "Engine/Rendering/DebugRendering/DebugRenderSystem.hpp"


//-----------------------------------------------------------------------------------------------
// Constructor
//
NPCSpawner::NPCSpawner(const Vector3& position, unsigned int teamIndex)
	: m_timeBetweenSpawns(DEFAULT_TIME_BETWEEN_SPAWNS)
{
	m_team = teamIndex;

	transform.position = position;

	m_stopwatch = new Stopwatch(Game::GetGameClock());
	m_stopwatch->SetInterval(m_timeBetweenSpawns);


	// To visualize it
	DebugRenderOptions options;
	options.m_isWireFrame = true;
	options.m_lifetime = 10000.f;
	options.m_renderMode = DEBUG_RENDER_XRAY;

	DebugRenderSystem::DrawCube(transform.position, options, Vector3(3.f));
}


//-----------------------------------------------------------------------------------------------
// Destructor
//
NPCSpawner::~NPCSpawner()
{
	delete m_stopwatch;
	m_stopwatch = nullptr;
}


//-----------------------------------------------------------------------------------------------
// Update
//
void NPCSpawner::Update(float deltaTime)
{
	if (m_stopwatch->HasIntervalElapsed())
	{
		SpawnEntity();
		m_stopwatch->SetInterval(m_timeBetweenSpawns);
	}

	GameObject::Update(deltaTime);
}


//-----------------------------------------------------------------------------------------------
// Sets the spawn rate for this spawner
//
void NPCSpawner::SetSpawnRatePerMinute(float spawnRatePerMinute)
{
	m_timeBetweenSpawns = (1.f / spawnRatePerMinute) * 60.f;
}


//-----------------------------------------------------------------------------------------------
// Spawns an NPC tank
//
void NPCSpawner::SpawnEntity() const
{
	NPCTank* tank = new NPCTank(m_team);
	tank->transform = transform;

	Game::GetMap()->AddNPCTank(tank);
}
