#include "Game/Entity/Tank.hpp"
#include "Game/Entity/Turret.hpp"
#include "Game/Framework/Game.hpp"
#include "Game/Environment/Map.hpp"
#include "Game/Framework/GameCommon.hpp"

#include "Engine/Assets/AssetDB.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Rendering/Core/Renderable.hpp"
#include "Engine/Rendering/Core/RenderScene.hpp"

Tank::Tank()
{
	// Set up the tank base renderable
	m_renderable = new Renderable();
	RenderableDraw_t draw;
	draw.sharedMaterial = AssetDB::GetSharedMaterial("Phong_Opaque");
	draw.mesh = AssetDB::GetMesh("Cube");

	draw.drawMatrix = Matrix44::MakeModelMatrix(Vector3(0.f, 0.5f, 0.f), Vector3::ZERO, Vector3(3.f, 1.3f, 4.f));
	
	m_renderable->AddDraw(draw);
	m_renderable->AddInstanceMatrix(transform.GetToWorldMatrix());

	Game::GetRenderScene()->AddRenderable(m_renderable);

	// Make the turret child to our transform
	m_turret = new Turret(transform);
}

Tank::~Tank()
{
	delete m_turret;
	m_turret = nullptr;

	Game::GetRenderScene()->RemoveRenderable(m_renderable);
	delete m_renderable;
	m_renderable = nullptr;
}

void Tank::Update(float deltaTime)
{
	GameObject::Update(deltaTime);

	UpdateHeightOnMap();
	UpdateOrientationWithNormal();

	m_renderable->SetInstanceMatrix(0, transform.GetToWorldMatrix());

	m_turret->Update(deltaTime);
}

void Tank::UpdateHeightOnMap()
{
	Map* map = Game::GetMap();
	float height = map->GetHeightAtPosition(transform.position);
	transform.position.y = height;
}

void Tank::UpdateOrientationWithNormal()
{
	Map* map = Game::GetMap();

	Vector3 normal = map->GetNormalAtPosition(transform.position);
	Vector3 right = CrossProduct(normal, transform.GetWorldForward());
	right.NormalizeAndGetLength();

	Vector3 newForward = CrossProduct(right, normal);
	newForward.NormalizeAndGetLength();

	Matrix44 newModel = Matrix44(right, normal, newForward, transform.position);
	transform.SetModelMatrix(newModel);
}

