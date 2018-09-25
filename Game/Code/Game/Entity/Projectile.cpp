#include "Game/Entity/Projectile.hpp"
#include "Game/Framework/Game.hpp"
#include "Engine/Core/Time/Stopwatch.hpp"

Projectile::Projectile()
{
	m_collisionRadius = 0.5f;
	m_stopwatch = new Stopwatch(Game::GetGameClock());

	SetupVoxelTextures("Data/3DTextures/TestProjectile.qef");
}

void Projectile::Update()
{
	DynamicEntity::Update();

	if (!IsMarkedForDelete() && m_stopwatch->HasIntervalElapsed())
	{
		m_isMarkedForDelete = true;
	}
}

void Projectile::OnCollision(Entity* other)
{
	DynamicEntity::OnCollision(other);
}

void Projectile::OnDamageTaken(int damageAmount)
{
	DynamicEntity::OnDamageTaken(damageAmount);
}

void Projectile::OnDeath()
{
	DynamicEntity::OnDeath();
}

void Projectile::OnSpawn()
{
	DynamicEntity::OnSpawn();

	m_stopwatch->SetInterval(m_lifetime);
}
