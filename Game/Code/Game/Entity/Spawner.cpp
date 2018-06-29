/************************************************************************/
/* File: NPCSpawner.cpp
/* Author: Andrew Chase
/* Date: June 18th, 2018
/* Description: Implementation of the NPCSpawner class
/************************************************************************/
#include "Game/Framework/App.hpp"
#include "Game/Framework/Game.hpp"
#include "Game/Entity/NPCTank.hpp"
#include "Game/Environment/Map.hpp"
#include "Game/Entity/Spawner.hpp"

#include "Engine/Assets/AssetDB.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/Time/Stopwatch.hpp"
#include "Engine/Rendering/Core/RenderScene.hpp"
#include "Engine/Rendering/Materials/MaterialInstance.hpp"

#include "Engine/Rendering/DebugRendering/DebugRenderSystem.hpp"


//-----------------------------------------------------------------------------------------------
// Constructor
//
Spawner::Spawner(const Vector3& position, unsigned int teamIndex)
	: GameEntity(ENTITY_SWARMER)
	, m_timeBetweenSpawns(DEFAULT_TIME_BETWEEN_SPAWNS)
{
	m_team = teamIndex;

	transform.position = position;

	m_stopwatch = new Stopwatch(Game::GetGameClock());
	m_stopwatch->SetInterval(0.f);


	// To visualize it
	DebugRenderOptions options;
	options.m_isWireFrame = true;
	options.m_lifetime = 10000.f;
	options.m_renderMode = DEBUG_RENDER_XRAY;

	m_physicsRadius = 2.f;

	m_renderable = new Renderable();

	RenderableDraw_t draw;
	draw.sharedMaterial = AssetDB::GetSharedMaterial("Data/Materials/Tank.material");
	draw.mesh = AssetDB::GetMesh("Cube");
	draw.drawMatrix = Matrix44::MakeModelMatrix(Vector3::ZERO, Vector3::ZERO, 3.f * Vector3::ONES);

	m_renderable->AddDraw(draw);
	m_renderable->AddInstanceMatrix(transform.GetWorldMatrix());

	Game::GetRenderScene()->AddRenderable(m_renderable);
}


//-----------------------------------------------------------------------------------------------
// Destructor
//
Spawner::~Spawner()
{
	Game::GetRenderScene()->RemoveRenderable(m_renderable);

	delete m_stopwatch;
	m_stopwatch = nullptr;

	if (!App::GetInstance()->IsQuitting())
	{
		AudioSystem::GetInstance()->PlaySoundFromAudioGroup("enemy.die");
	}
}


void Spawner::OnCollisionWithEntity(GameEntity* other)
{
	if (other->GetTeamIndex() != m_team)
	{
		TakeDamage(1000000);
	}

	GameEntity::OnCollisionWithEntity(other);
}

//-----------------------------------------------------------------------------------------------
// Update
//
void Spawner::Update(float deltaTime)
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
void Spawner::SetSpawnRatePerMinute(float spawnRatePerMinute)
{
	m_timeBetweenSpawns = (1.f / spawnRatePerMinute) * 60.f;
}
