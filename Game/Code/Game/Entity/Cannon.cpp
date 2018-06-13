#include "Game/Entity/Cannon.hpp"
#include "Game/Framework/Game.hpp"
#include "Game/Framework/GameCommon.hpp"

#include "Engine/Math/Matrix44.hpp"
#include "Engine/Assets/AssetDB.hpp"
#include "Engine/Rendering/Core/Renderable.hpp"
#include "Engine/Rendering/Core/RenderScene.hpp"

Cannon::Cannon(Transform& parent)
{
	transform.SetParentTransform(&parent);

	// Set up the cannon renderable
	m_renderable = new Renderable();
	RenderableDraw_t draw;
	draw.sharedMaterial = AssetDB::GetSharedMaterial("Phong_Opaque");
	draw.mesh = AssetDB::GetMesh("Cube");
	draw.drawMatrix = Matrix44::MakeModelMatrix(Vector3(0.f, 0.25f, 1.5f), Vector3::ZERO, Vector3(0.2f, 0.2f, 2.f));

	m_renderable->AddDraw(draw);
	m_renderable->AddInstanceMatrix(transform.GetToWorldMatrix());

	Game::GetRenderScene()->AddRenderable(m_renderable);
}

Cannon::~Cannon()
{
	Game::GetRenderScene()->RemoveRenderable(m_renderable);
	delete m_renderable;
	m_renderable = nullptr;
}


void Cannon::Update(float deltaTime)
{
	UNUSED(deltaTime);
	m_renderable->SetInstanceMatrix(0, transform.GetToWorldMatrix());
}

void Cannon::ElevateTowardsTarget(const Vector3& target)
{

}
