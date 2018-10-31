/************************************************************************/
/* File: Item.cpp
/* Author: Andrew Chase
/* Date: October 29th 2018
/* Description: 
/************************************************************************/
#include "Game/Entity/Item.hpp"
#include "Game/Entity/Player.hpp"
#include "Game/Framework/Game.hpp"
#include "Game/Framework/World.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Game/Entity/Components/PhysicsComponent.hpp"

//-----------------------------------------------------------------------------------------------
// Constructor
//
Item::Item(const EntityDefinition* definition)
	: Entity(definition)
{
	m_entityTeam = ENTITY_TEAM_PLAYER;
}

void Item::Update()
{
	World* world = Game::GetWorld();

	if (world->IsEntityOnGround(this))
	{
		m_physicsEnabled = false;
		world->SnapEntityToGround(this);
		m_basePosition = m_position;
	}

	// Have the item bob/spin while on the ground (not simulating physics)
	if (!m_physicsEnabled)
	{
		float time = m_bobTimer.GetElapsedTime();

		float offset = 1.0f + SinDegrees(time * ITEM_BOB_RATE);

		m_position = m_basePosition + Vector3(0.f, offset, 0.f);
		m_orientation += ITEM_SPIN_RATE * m_bobTimer.GetDeltaSeconds();
	}
}

void Item::OnCollision(Entity* other)
{
	if (other->IsPlayer())
	{
		Player* player = dynamic_cast<Player*>(other);
		player->AddItemSet(m_definition->m_initialItems);

		OnDeath();
	}
}

void Item::OnDeath()
{
	Entity::OnDeath();
	Game::GetWorld()->ParticalizeEntity(this);
}

void Item::OnSpawn()
{
	Entity::OnSpawn();
	m_bobTimer.Reset();

	Vector3 initialVelocity = Vector3(GetRandomFloatInRange(7.f, 10.f), 75.f, GetRandomFloatInRange(7.f, 10.f));

	if (CheckRandomChance(0.5f))
	{
		initialVelocity.x *= -1.0f;
	}

	if (CheckRandomChance(0.5f))
	{
		initialVelocity.z *= -1.0f;
	}

	m_physicsComponent->SetVelocity(initialVelocity);
}

