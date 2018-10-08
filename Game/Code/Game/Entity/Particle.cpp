#include "Game/Framework/Game.hpp"
#include "Game/Entity/Particle.hpp"

Particle::Particle(const Rgba& color, float lifetime, const Vector3& position, const Vector3& initialVelocity)
	: Entity(EntityDefinition::GetDefinition("Particle"))
{
// 	m_position = position;
// 	m_velocity = initialVelocity;
// 	m_lifetime = lifetime;
// 
// 	m_affectedByGravity = true;
// 
// 	m_stopwatch.SetClock(Game::GetGameClock());
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
