/************************************************************************/
/* File: Zombie.cpp
/* Author: Andrew Chase
/* Date: May 9th, 2018
/* Description: Implementation of the Zombie class
/************************************************************************/
#include "Game/Zombie.hpp"
#include "Game/Player.hpp"
#include "Game/GameState_Playing.hpp"

#include "Engine/Core/AssetDB.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/Sampler.hpp"
#include "Engine/Renderer/Material.hpp"
#include "Engine/Renderer/Renderable.hpp"
#include "Engine/Renderer/RenderScene.hpp"

// Class constants
float Zombie::s_turnSpeed = Zombie::ZOMBIE_INITIAL_TURN_SPEED;
float Zombie::s_walkSpeed = Zombie::ZOMBIE_INITIAL_WALK_SPEED;


//-----------------------------------------------------------------------------------------------
// Constructor
//
Zombie::Zombie()
{
	transform.position = 10.f * GetRandomPointOnSphere();
	transform.position.y = 0.f;
	transform.rotation = Vector3(0.f, GetRandomFloatInRange(0.f, 360.f), 0.f);

	m_renderable = new Renderable(transform.GetModelMatrix(), AssetDB::CreateOrGetMeshGroup("Miku.obj"), nullptr);

	Material* quad		= AssetDB::CreateOrGetSharedMaterial("Miku_Quad"); 
	Material* base		= AssetDB::CreateOrGetSharedMaterial("Miku_Base"); 
	Material* detail	= AssetDB::CreateOrGetSharedMaterial("Miku_Detail");

	// Make a linear sampler for miku
	Sampler* sampler = new Sampler();
	sampler->Initialize(SAMPLER_FILTER_LINEAR, EDGE_SAMPLING_REPEAT);
	
	quad->SetSampler(0, sampler);
	base->SetSampler(0, sampler);
	detail->SetSampler(0, sampler);
	quad->SetProperty("SPECULAR_POWER", 25.0f);
	quad->SetProperty("SPECULAR_AMOUNT", 1.f);
	base->SetProperty("SPECULAR_POWER", 25.0f);
	base->SetProperty("SPECULAR_AMOUNT", 1.f);	
	detail->SetProperty("SPECULAR_POWER", 25.0f);
	detail->SetProperty("SPECULAR_AMOUNT", 1.f);
	
	m_renderable->SetSharedMaterial(quad, 0);
	m_renderable->SetSharedMaterial(base, 1);
	m_renderable->SetSharedMaterial(detail, 2);
	m_renderable->SetSharedMaterial(base, 3);

	GameState_Playing::GetRenderScene()->AddRenderable(m_renderable);
}


//-----------------------------------------------------------------------------------------------
// Base destructor
//
Zombie::~Zombie()
{
	GameState_Playing::GetRenderScene()->RemoveRenderable(m_renderable);

	delete m_renderable;
	m_renderable = nullptr;

	// Every death makes them faster....
	s_turnSpeed += 1.5f;
	s_walkSpeed += 0.01f;
}


//-----------------------------------------------------------------------------------------------
// Update
//
void Zombie::Update(float deltaTime)
{
	GameObject::Update(deltaTime);

	Player* player = GameState_Playing::GetPlayer();
	m_targetPosition = player->transform.position;

	// Turn towards the target
	Vector3 directionToTarget = (m_targetPosition - transform.position).GetNormalized();
	Vector2 dirXZ = directionToTarget.xz();
	float orientation = 180.f - (dirXZ.GetOrientationDegrees() + 90.f);
	float newAmount = TurnToward(transform.rotation.y, orientation, s_turnSpeed * deltaTime);

	transform.rotation = (Vector3(0.f, newAmount, 0.f));

	// Walk in local forward
	Vector3 velocity = Vector3(0.f, 0.f, 1.f) * s_walkSpeed;
	transform.TranslateLocal(velocity);

	// Update the renderable
	m_renderable->SetModelMatrix(transform.GetModelMatrix(), 0);
}
