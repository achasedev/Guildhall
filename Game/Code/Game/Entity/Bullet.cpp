/************************************************************************/
/* File: Bullet.cpp
/* Author: Andrew Chase
/* Date: June 18th, 2018
/* Description: Implementation of the Bullet class
/************************************************************************/
#include "Game/Entity/Bullet.hpp"
#include "Game/Framework/Game.hpp"
#include "Game/Framework/GameCommon.hpp"

#include "Engine/Assets/AssetDB.hpp"
#include "Engine/Rendering/Core/RenderScene.hpp"

// Constants
const float Bullet::BULLET_SPEED = 5.f;
const float Bullet::BULLET_LIFETIME = 5.f;


//-----------------------------------------------------------------------------------------------
// Constructor
//
Bullet::Bullet(const Vector3& position, const Quaternion& orientation, unsigned int team)
	: GameEntity(ENTITY_BULLET)
	, m_timeToLive(BULLET_LIFETIME)
{
	m_team = team;

	// Set the transform up to spawn at the position and orientation
	transform.position = position;
	transform.rotation = orientation;

	// Set up the renderable
	m_renderable = new Renderable();
	RenderableDraw_t draw;
	draw.sharedMaterial = AssetDB::GetSharedMaterial("Default_Opaque");
	draw.mesh = AssetDB::GetMesh("Cube");

	m_renderable->AddDraw(draw);
	m_renderable->AddInstanceMatrix(transform.GetWorldMatrix());

	Game::GetRenderScene()->AddRenderable(m_renderable);
}


//-----------------------------------------------------------------------------------------------
// Destructor
//
Bullet::~Bullet()
{
	Game::GetRenderScene()->RemoveRenderable(m_renderable);

	delete m_renderable;
	m_renderable = nullptr;
}


//-----------------------------------------------------------------------------------------------
// Update
//
void Bullet::Update(float deltaTime)
{
	m_timeToLive -= deltaTime;

	if (m_timeToLive <= 0.f)
	{
		m_markedForDelete = true;
	}

	transform.TranslateLocal(Vector3::DIRECTION_FORWARD * BULLET_SPEED);
	m_renderable->SetInstanceMatrix(0, transform.GetWorldMatrix());
}


//-----------------------------------------------------------------------------------------------
// Returns the amount of damage this bullet inflicts
//
unsigned int Bullet::GetDamageAmount() const
{
	return m_damageAmount;
}
