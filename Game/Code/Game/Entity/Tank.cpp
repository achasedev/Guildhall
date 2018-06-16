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

const float Tank::TANK_ROTATION_SPEED = 60.f;
const float Tank::TANK_TRANSLATION_SPEED = 5.f;
const float Tank::TANK_DEFAULT_FIRERATE = 1.f;

Tank::Tank()
{
	m_fireRate = TANK_DEFAULT_FIRERATE;
	// Set up the tank base renderable
	m_renderable = new Renderable();
	RenderableDraw_t draw;
	draw.sharedMaterial = AssetDB::GetSharedMaterial("Phong_Opaque");
	draw.mesh = AssetDB::GetMesh("Cube");

	draw.drawMatrix = Matrix44::MakeModelMatrix(Vector3(0.f, 0.5f, 0.f), Vector3::ZERO, Vector3(3.f, 1.3f, 4.f));
	
	m_renderable->AddDraw(draw);
	m_renderable->AddInstanceMatrix(transform.GetWorldMatrix());

	Game::GetRenderScene()->AddRenderable(m_renderable);

	// Make the turret child to our transform
	m_turret = new Turret(transform);
	m_stopwatch = new Stopwatch(Game::GetGameClock());
	m_stopwatch->SetInterval(1.f / m_fireRate);
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

	m_renderable->SetInstanceMatrix(0, transform.GetWorldMatrix());

	if (m_hasTarget && m_lookAtTarget)
	{
		m_turret->TurnTowardsTarget(m_target);
	}

	m_turret->Update(deltaTime);
}

void Tank::SetTarget(bool hasTarget, const Vector3& target /*= Vector3::ZERO*/)
{
	m_hasTarget = hasTarget;
	m_target = target;
}

void Tank::ShootCannon()
{
	if (m_stopwatch->HasIntervalElapsed())
	{
		Matrix44 fireTransform = m_turret->GetCannon()->GetFireTransform();
		Vector3 position = Matrix44::ExtractTranslation(fireTransform);
		Quaternion rotation = Quaternion::FromEuler(Matrix44::ExtractRotationDegrees(fireTransform));

		Bullet* bullet = new Bullet(position, rotation);
		Game::AddGameObject(bullet);

		m_stopwatch->SetInterval(1.f / m_fireRate);
	}
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

