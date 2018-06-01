/************************************************************************/
/* File: Bullet.cpp
/* Author: Andrew Chase
/* Date: May 9th, 2018
/* Description: Implementation of the Bullet class
/************************************************************************/
#include "Game/Game.hpp"
#include "Game/Bullet.hpp"
#include "Game/GameState_Playing.hpp"

#include "Engine/Core/AssetDB.hpp"
#include "Engine/Renderer/Light.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/Renderable.hpp"
#include "Engine/Renderer/MeshBuilder.hpp"
#include "Engine/Renderer/RenderScene.hpp"
#include "Engine/Renderer/ParticleEmitter.hpp"
#include "Engine/Renderer/MaterialInstance.hpp"


//-----------------------------------------------------------------------------------------------
// Constructor
//
Bullet::Bullet(const Transform& startingTransform)
{
	transform = startingTransform;
	transform.position += transform.GetWorldForward() * 3.f;

	m_velocity = transform.GetWorldForward() * BULLET_SPEED;
	m_timeToLive = BULLET_LIFETIME;

	// Set up the renderable
	Mesh* bulletMesh = AssetDB::GetMesh("Bullet");
	if (bulletMesh == nullptr)
	{
		MeshBuilder mb;
		mb.BeginBuilding(PRIMITIVE_TRIANGLES, true);
		mb.PushCube(Vector3::ZERO, Vector3(0.1f, 0.1f, 1.f), Rgba::RED);
		mb.FinishBuilding();
		bulletMesh = mb.CreateMesh();

		AssetDB::AddMesh("Bullet", bulletMesh);
	}

	Material* material = AssetDB::CreateOrGetSharedMaterial("Default_Opaque");
	m_renderable = new Renderable(transform.GetModelMatrix(), bulletMesh, material); 

	// Make the trail emitter
	m_trailEmitter = new ParticleEmitter(Game::GetGameClock());
	m_trailEmitter->SetTransform(transform.position, transform.rotation, transform.scale);

	Renderable* particleRend = new Renderable();
	Material* sharedMat = AssetDB::CreateOrGetSharedMaterial("Default_Opaque");
	MaterialInstance* instanceMat = AssetDB::CreateMaterialInstance("Default_Opaque");
	instanceMat->SetShader(AssetDB::CreateOrGetShader("Default_Opaque_Instanced"), false);

	MaterialMeshSet set;
	set.m_mesh = AssetDB::GetMesh("Particle");
	set.m_sharedMaterial = sharedMat;
	set.m_materialInstance = instanceMat;
	particleRend->SetMaterialMeshSet(0, set);
	
	m_trailEmitter->SetRenderable(particleRend);
	m_trailEmitter->SetSpawnRate(20);
	m_trailEmitter->SetSpawnLifetimeFunction([]() { return 0.5f; });
	m_trailEmitter->SetSpawnVelocityFunction([]() { return 2.f * GetRandomPointOnSphere(); });
	m_trailEmitter->SetKillWhenDone(true);

	GameState_Playing::GetRenderScene()->AddRenderable(particleRend);

	m_light = Light::CreatePointLight(transform.position, Rgba(255, 0, 0, 100), Vector3(0.f, 0.f, 0.001f));
	GameState_Playing::GetRenderScene()->AddLight(m_light);
}


//-----------------------------------------------------------------------------------------------
// Destructor - cleans up renderables and lights
//
Bullet::~Bullet()
{
	GameState_Playing::GetRenderScene()->RemoveRenderable(m_renderable);

	delete m_renderable;
	m_renderable = nullptr;

	// Remove the particle renderable before deletion!
	GameState_Playing::GetRenderScene()->RemoveRenderable(m_trailEmitter->GetRenderable());

	delete m_trailEmitter;
	m_trailEmitter = nullptr;

	GameState_Playing::GetRenderScene()->RemoveLight(m_light);
	delete m_light;
	m_light = nullptr;
}


//-----------------------------------------------------------------------------------------------
// Update
//
void Bullet::Update(float deltaTime)
{
	GameObject::Update(deltaTime);

	// Move the bullet
	transform.position += m_velocity * deltaTime;

	// Update the renderable location
	m_renderable->SetModelMatrix(transform.GetModelMatrix(), 0);
	m_trailEmitter->transform = transform;
	m_trailEmitter->Update();

	// Update the light position
	m_light->SetPosition(transform.position);

	m_timeToLive -= deltaTime;
	if (m_timeToLive <= 0.f)
	{
		SetMarkedForDelete(true);
	}
}
