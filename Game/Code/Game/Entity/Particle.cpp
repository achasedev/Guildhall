#include "Game/Framework/Game.hpp"
#include "Game/Entity/Particle.hpp"

Particle::Particle(const Rgba& color, float lifetime, const Vector3& position, const Vector3& initialVelocity)
	: DynamicEntity()
{
	m_textures[0] = new VoxelTexture();
	m_textures[0]->CreateFromColorStream(&color, IntVector3(1, 1, 1));

	m_textures[1] = m_textures[0]->Clone();
	m_textures[2] = m_textures[0]->Clone();
	m_textures[3] = m_textures[0]->Clone();

	m_collisionDef = CollisionDefinition_t(COLLISION_SHAPE_BOX, COLLISION_RESPONSE_FULL_CORRECTION, 0.5f, 1.0f, 0.5f);

	m_position = position;
	m_velocity = initialVelocity;
	m_lifetime = lifetime;

	m_affectedByGravity = true;

	m_stopwatch.SetClock(Game::GetGameClock());
}

Particle::~Particle()
{

}

void Particle::Update()
{
	if (m_stopwatch.HasIntervalElapsed())
	{
		m_isMarkedForDelete = true;
	}
}

void Particle::OnDeath()
{
}

void Particle::OnSpawn()
{
	m_stopwatch.SetInterval(m_lifetime);
}

