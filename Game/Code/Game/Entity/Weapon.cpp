#include "Game/Entity/Weapon.hpp"
#include "Game/Entity/Player.hpp"
#include "Game/Framework/Game.hpp"
#include "Game/Framework/World.hpp"
#include "Game/Entity/Projectile.hpp"
#include "Game/Framework/GameCommon.hpp"
#include "Game/Entity/Components/PhysicsComponent.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Time/Stopwatch.hpp"


Weapon::Weapon(const EntityDefinition* definition)
	: Item(definition)
	, m_shootTimer(Stopwatch(Game::GetGameClock()))
{
}

void Weapon::OnEntityCollision(Entity* other)
{
	Item::OnEntityCollision(other);

	if (other->IsPlayer())
	{
		Player* player = dynamic_cast<Player*>(other);
		player->EquipWeapon(this);

		Game::GetWorld()->RemoveEntity(this);
	}
}

void Weapon::OnEquip(Player* playerEquipping)
{
	UNUSED(playerEquipping);

	m_playerEquippedTo = playerEquipping;

	m_shootTimer.SetInterval(1.f / m_definition->m_fireRate);
}

void Weapon::Shoot()
{
	if (m_playerEquippedTo == nullptr)
	{
		return;
	}

	// Check the shoot timer
	if (m_shootTimer.DecrementByIntervalAll() == 0)
	{
		return;
	}

	Vector3 baseDirection = m_playerEquippedTo->GetForwardVector();
	Vector3 position = m_playerEquippedTo->GetCenterPosition() + (baseDirection * WEAPON_FIRE_OFFSET);
	eEntityTeam team = m_playerEquippedTo->GetTeam();
	float projectileSpeed = m_definition->m_projectileDefinition->m_projectileSpeed;
	World* world = Game::GetWorld();

	int projectileCount = m_definition->m_projectilesFiredPerShot;

	// Shoot as many as we are supposed to this fire
	for (int i = 0; i < projectileCount; ++i)
	{
		Projectile* proj = new Projectile(m_definition->m_projectileDefinition);

		Vector3 spread;
		spread.x = GetRandomFloatInRange(-m_definition->m_fireSpread, m_definition->m_fireSpread);
		spread.y = GetRandomFloatInRange(-m_definition->m_fireSpread, m_definition->m_fireSpread);
		spread.z = GetRandomFloatInRange(-m_definition->m_fireSpread, m_definition->m_fireSpread);

		Vector3 finalDirection = baseDirection + spread;
		float finalOrientation = finalDirection.xz().GetOrientationDegrees();

		proj->SetPosition(position);
		proj->SetOrientation(finalOrientation);
		proj->SetTeam(team);

		proj->GetPhysicsComponent()->SetVelocity(finalDirection * projectileSpeed);

		world->AddEntity(proj);
	}
}

