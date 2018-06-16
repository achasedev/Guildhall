#include "Game/Entity/Cannon.hpp"
#include "Game/Framework/Game.hpp"
#include "Game/Framework/GameCommon.hpp"

#include "Engine/Math/Matrix44.hpp"
#include "Engine/Assets/AssetDB.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Rendering/Core/Renderable.hpp"
#include "Engine/Rendering/Core/RenderScene.hpp"

const float Cannon::CANNON_ROTATION_SPEED = 20.f;

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
	m_renderable->AddInstanceMatrix(transform.GetWorldMatrix());

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
	m_renderable->SetInstanceMatrix(0, transform.GetWorldMatrix());
}

void Cannon::ElevateTowardsTarget(const Vector3& target)
{
	Matrix44 toParent = transform.GetLocalMatrix();
	Matrix44 toLocal = transform.GetWorldMatrix().GetInverse();

	Vector3 localPosition = toLocal.TransformPoint(target).xyz();
	localPosition.x = 0.f;

	Vector3 parentPosition = toParent.TransformPoint(localPosition).xyz();
	Vector3 currentRotation = transform.rotation.GetAsEulerAngles();

	Vector3 dirToTarget3D = (parentPosition - transform.position);
	Vector2 dirToTarget = Vector2(dirToTarget3D.z, dirToTarget3D.y);

	float startAngle = currentRotation.x;
	float endAngle = -dirToTarget.GetOrientationDegrees();

	float deltaTime = Game::GetDeltaTime();
	float newAngle = TurnToward(startAngle, endAngle, CANNON_ROTATION_SPEED * deltaTime);

	currentRotation.x = newAngle;
	transform.rotation = Quaternion::FromEuler(currentRotation);
}
