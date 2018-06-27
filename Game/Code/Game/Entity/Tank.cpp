/************************************************************************/
/* File: Tank.cpp
/* Author: Andrew Chase
/* Date: June 18th, 2018
/* Description: Implementation of the Tank class
/************************************************************************/
#include "Game/Entity/Tank.hpp"
#include "Game/Entity/Bullet.hpp"
#include "Game/Entity/Cannon.hpp"
#include "Game/Entity/Turret.hpp"
#include "Game/Framework/Game.hpp"
#include "Game/Environment/Map.hpp"
#include "Game/Framework/GameCommon.hpp"

#include "Engine/Assets/AssetDB.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Time/Stopwatch.hpp"
#include "Engine/Rendering/Core/Renderable.hpp"
#include "Engine/Rendering/Core/RenderScene.hpp"

#include "Engine/Rendering/DebugRendering/DebugRenderSystem.hpp"

// Constants
const float Tank::TANK_ROTATION_SPEED = 60.f;
const float Tank::TANK_TRANSLATION_SPEED = 5.f;
const float Tank::TANK_DEFAULT_FIRERATE = 1.f;


//-----------------------------------------------------------------------------------------------
// Constructor
//
Tank::Tank(unsigned int team)
	: GameEntity(ENTITY_TANK)
{
	m_team = team;
	m_health = 10;

	m_shouldStickToTerrain = true;
	m_shouldOrientToTerrain = true;

	m_fireRate = TANK_DEFAULT_FIRERATE;
	// Set up the tank base renderable
	m_renderable = new Renderable();
	RenderableDraw_t draw;
	draw.sharedMaterial = AssetDB::GetSharedMaterial("Data/Materials/Tank.material");
	draw.mesh = AssetDB::GetMesh("Cube");

	draw.drawMatrix = Matrix44::MakeModelMatrix(Vector3(0.f, 0.5f, 0.f), Vector3::ZERO, Vector3(3.f, 1.3f, 4.f));
	
	m_renderable->AddDraw(draw);
	m_renderable->AddInstanceMatrix(transform.GetWorldMatrix());

	Game::GetRenderScene()->AddRenderable(m_renderable);

	// Make the turret child to our transform
	m_turret = new Turret(transform);
	m_stopwatch = new Stopwatch(Game::GetGameClock());
	m_stopwatch->SetInterval(1.f / m_fireRate);

	// Set physics radius
	m_physicsRadius = 3.0f;
}


//-----------------------------------------------------------------------------------------------
// Destructor
//
Tank::~Tank()
{
	delete m_turret;
	m_turret = nullptr;

	Game::GetRenderScene()->RemoveRenderable(m_renderable);
	delete m_renderable;
	m_renderable = nullptr;
}


//-----------------------------------------------------------------------------------------------
// Update
//
void Tank::Update(float deltaTime)
{
	GameEntity::Update(deltaTime);

	m_renderable->SetInstanceMatrix(0, transform.GetWorldMatrix());

	if (m_hasTarget && m_lookAtTarget)
	{
		m_turret->TurnTowardsTarget(m_target);
	}

	m_turret->Update(deltaTime);
}


void Tank::OnCollisionWithEntity(GameEntity* other)
{
	GameEntity::OnCollisionWithEntity(other);
}

//-----------------------------------------------------------------------------------------------
// Sets the target to the one specified, or sets the flag for no target
//
void Tank::SetTarget(bool hasTarget, const Vector3& target /*= Vector3::ZERO*/)
{
	m_hasTarget = hasTarget;
	m_target = target;
}


//-----------------------------------------------------------------------------------------------
// Additively applies the damageAmount to this tank's health
void Tank::TakeDamage(int damageAmount)
{
	m_health -= damageAmount;

	if (m_health <= 0)
	{
		SetMarkedForDelete(true);
	}
}


//-----------------------------------------------------------------------------------------------
// Sets the tank's health to the one specified
//
void Tank::SetHealth(int health)
{
	m_health = health;
}


//-----------------------------------------------------------------------------------------------
// Spawns a bullet if the shoot cooldown is finished
//
void Tank::ShootCannon()
{
	if (m_stopwatch->HasIntervalElapsed())
	{
		Matrix44 fireTransform = m_turret->GetCannon()->GetFireTransform();
		Vector3 position = Matrix44::ExtractTranslation(fireTransform);
		Quaternion rotation = Quaternion::FromEuler(Matrix44::ExtractRotationDegrees(fireTransform));

		Bullet* bullet = new Bullet(position, rotation, m_team);
		Game::GetMap()->AddGameEntity(bullet);

		m_stopwatch->SetInterval(1.f / m_fireRate);
	}
}


//-----------------------------------------------------------------------------------------------
// Returns the time until the tank can fire the next shot
//
float Tank::GetTimeUntilNextShot() const
{
	return m_stopwatch->GetTimeUntilIntervalEnd();
}
