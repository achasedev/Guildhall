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

void Turret::TurnTowardsTarget(const Vector3& target)
{
// 	Vector3 worldDirection = (target - transform.position).GetNormalized();
// 	Vector3 directionInParentSpace = transform.GetParentsToWorldMatrix().TransformVector(worldDirection).xyz();
// 
// 	Matrix44 lookAt = Matrix44::MakeLookAt(transform.position, directionInParentSpace, transform.GetWorldUp());
// 
// 	Vector3 oldRotation = transform.rotation;
// 	Matrix44 result = Interpolate(lookAt, transform.GetToWorldMatrix(), 0.25f);
// 
// 	Vector3 newRotation = Matrix44::ExtractRotationDegrees(result);
// 	newRotation.y = oldRotation.y;
// 
// 
// 	m_cannon->ElevateTowardsTarget(target);
}
