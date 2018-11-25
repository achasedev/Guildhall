#include "Game/Entity/Components/BehaviorComponent_Shoot.hpp"
#include "Game/Entity/Player.hpp"
#include "Game/Framework/World.hpp"
#include "Game/Framework/Game.hpp"
#include "Game/Entity/Projectile.hpp"
#include "Game/Entity/Components/PhysicsComponent.hpp"
#include "Engine/Core/Utility/ErrorWarningAssert.hpp"

BehaviorComponent_Shoot::BehaviorComponent_Shoot(const EntityDefinition* projectileDef, float fireRate)
	: m_projectileDefinition(projectileDef)
	, m_fireRate(fireRate)
	, m_shootInterval(1.0f / fireRate)
{
	m_shootTimer.SetInterval(m_shootInterval);
}

void BehaviorComponent_Shoot::Update()
{
	UNIMPLEMENTED();
}

BehaviorComponent* BehaviorComponent_Shoot::Clone() const
{
	ASSERT_OR_DIE(m_owningEntity == nullptr, "Error: Behavior clone had non-null base members on prototype");
	return new BehaviorComponent_Shoot(m_projectileDefinition, m_fireRate);
}

void BehaviorComponent_Shoot::Shoot()
{
	Vector3 entityPosition = m_owningEntity->GetPosition();
	float entityOrientation = m_owningEntity->GetOrientation();

	Projectile* proj = new Projectile(EntityDefinition::GetDefinition("Bullet"), m_owningEntity->GetTeam());
	proj->SetPosition(entityPosition + Vector3(0.f, 4.f, 0.f));
	proj->SetOrientation(entityOrientation);
	proj->SetTeam(m_owningEntity->GetTeam());

	Vector2 direction = Vector2::MakeDirectionAtDegrees(entityOrientation);
	proj->GetPhysicsComponent()->SetVelocity(Vector3(direction.x, 0.f, direction.y) * 100.f);

	World* world = Game::GetWorld();
	world->AddEntity(proj);

	m_shootTimer.SetInterval(m_shootInterval);
}

