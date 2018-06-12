#include "Game/Entity/Turret.hpp"
#include "Game/Entity/Cannon.hpp"
#include "Game/Framework/Game.hpp"
#include "Game/Framework/GameCommon.hpp"

#include "Engine/Math/Matrix44.hpp"
#include "Engine/Assets/AssetDB.hpp"
#include "Engine/Rendering/Core/Renderable.hpp"
#include "Engine/Rendering/Core/RenderScene.hpp"

Turret::Turret(Transform& parent)
{
	transform.SetParentTransform(&parent);
	transform.position = Vector3(0.f, 1.3f, 0.f);

	// Set up the tank base renderable
	m_renderable = new Renderable();
	RenderableDraw_t draw;
	draw.sharedMaterial = AssetDB::GetSharedMaterial("Phong_Opaque");
	draw.mesh = AssetDB::GetMesh("Cube");
	draw.drawMatrix = Matrix44::MakeModelMatrix(Vector3::ZERO, Vector3::ZERO, Vector3(1.3f, 0.9f, 1.3f));

	m_renderable->AddDraw(draw);
	m_renderable->AddInstanceMatrix(transform.GetToWorldMatrix());

	Game::GetRenderScene()->AddRenderable(m_renderable);

	// Make the cannon child to our transform
	m_cannon = new Cannon(transform);
}

Turret::~Turret()
{
	delete m_cannon;
	m_cannon = nullptr;

	Game::GetRenderScene()->RemoveRenderable(m_renderable);
	delete m_renderable;
	m_renderable = nullptr;
}

void Turret::Update(float deltaTime)
{
	UNUSED(deltaTime);
	m_renderable->SetInstanceMatrix(0, transform.GetToWorldMatrix());

	m_cannon->Update(deltaTime);
}
