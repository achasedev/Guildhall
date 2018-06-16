#include "Game/Entity/Bullet.hpp"
#include "Game/Framework/Game.hpp"
#include "Engine/Assets/AssetDB.hpp"
#include "Engine/Rendering/Core/RenderScene.hpp"

const float Bullet::BULLET_SPEED = 5.f;

Bullet::Bullet(const Vector3& position, const Quaternion& orientation)
{
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

Bullet::~Bullet()
{
	Game::GetRenderScene()->RemoveRenderable(m_renderable);

	delete m_renderable;
	m_renderable = nullptr;
}

void Bullet::Update(float deltaTime)
{
	transform.TranslateLocal(Vector3::DIRECTION_FORWARD * BULLET_SPEED);

	m_renderable->SetInstanceMatrix(0, transform.GetWorldMatrix());
}
