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
	BehaviorComponent::Update();

	Player** players = Game::GetPlayers();
	World* world = Game::GetWorld();

	Vector3 closestPlayerPosition;
	float minDistance = 9999.f;
	bool playerFound = false;
	bool haveFoundPlayerInLineOfSight = false;
	Vector3 currentPosition = m_owningEntity->GetEntityPosition();

	for (int i = 0; i < MAX_PLAYERS; ++i)
	{
		if (players[i] != nullptr)
		{
			Vector3 playerPosition = players[i]->GetEntityPosition();
			float currDistance = (playerPosition - currentPosition).GetLengthSquared();

			// Update our target if...
			// 1. Haven't found a player yet
			// 2. We have found a player already, but this player is closer and...
			//  a. We don't have line of sight to our best yet, so this one is strictly better (and could have line of sight)
			//  b. We have line of sight on our best, but we also have line of sight on this one
			if (!playerFound || ((currDistance < minDistance) && (!haveFoundPlayerInLineOfSight || world->HasLineOfSight(currentPosition, playerPosition))))
			{
				minDistance = currDistance;
				closestPlayerPosition = playerPosition;
				playerFound = true;

				if (world->HasLineOfSight(currentPosition, playerPosition))
				{
					haveFoundPlayerInLineOfSight = true;
				}
			}
		}
	}

	// Shouldn't happen, but to avoid unidentifiable behavior
	if (!playerFound)
	{
		return;
	}

	// If we can't see a player, then path find to get to the closest one
	if (!haveFoundPlayerInLineOfSight)
	{
		Vector3 nextPosition = world->GetNextPositionTowardsPlayer(m_owningEntity->GetEntityPosition());
		Vector2 toNext = (nextPosition - m_owningEntity->GetEntityPosition()).GetNormalized().xz();

		m_owningEntity->Move(toNext);
	}
	else
	{
		// We can see a player, so move directly to them and shoot at them
		Vector3 directionToMove = (closestPlayerPosition - currentPosition).GetNormalized();
		m_owningEntity->Move(directionToMove.xz());

		if (m_shootTimer.HasIntervalElapsed())
		{
			Shoot();
		}
	}
}

BehaviorComponent* BehaviorComponent_Shoot::Clone() const
{
	ASSERT_OR_DIE(m_owningEntity == nullptr, "Error: Behavior clone had non-null base members on prototype");
	return new BehaviorComponent_Shoot(m_projectileDefinition, m_fireRate);
}

void BehaviorComponent_Shoot::Shoot()
{
	Vector3 entityPosition = m_owningEntity->GetEntityPosition();
	float entityOrientation = m_owningEntity->GetOrientation();

	Projectile* proj = new Projectile(EntityDefinition::GetDefinition("Bullet"));
	proj->SetPosition(entityPosition + Vector3(0.f, 4.f, 0.f));
	proj->SetOrientation(entityOrientation);
	proj->SetTeam(m_owningEntity->GetTeam());

	Vector2 direction = Vector2::MakeDirectionAtDegrees(entityOrientation);
	proj->GetPhysicsComponent()->SetVelocity(Vector3(direction.x, 0.f, direction.y) * 100.f);

	World* world = Game::GetWorld();
	world->AddEntity(proj);

	m_shootTimer.SetInterval(m_shootInterval);
}

