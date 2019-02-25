/************************************************************************/
/* File: AIEntity.cpp
/* Author: Andrew Chase
/* Date: October 20th 2018
/* Description: Implementation of the AIEntity class
/************************************************************************/
#include "Game/Entity/Item.hpp"
#include "Game/Entity/Weapon.hpp"
#include "Game/Framework/Game.hpp"
#include "Game/Entity/AIEntity.hpp"
#include "Game/Framework/World.hpp"
#include "Game/Entity/Components/BehaviorComponent.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Audio/AudioSystem.hpp"

//-----------------------------------------------------------------------------------------------
// Constructor
//
AIEntity::AIEntity(const EntityDefinition* definition)
	: AnimatedEntity(definition)
{
	// Initialize the Behavior Component
	m_behaviorComponent = definition->CloneBehaviorPrototype();
	m_behaviorComponent->Initialize(this);
}


//-----------------------------------------------------------------------------------------------
// Update
//
void AIEntity::Update()
{
	AnimatedEntity::Update();
	m_behaviorComponent->Update();
}


//-----------------------------------------------------------------------------------------------
// Death event, removes itself from the spawner's list of entities it spawned. Also particalizes
//
void AIEntity::OnDeath()
{
	AnimatedEntity::OnDeath();

	m_eventSpawnedFrom->StopTrackingEntity(this);
	Game::GetWorld()->ParticalizeEntity(this);

	if (CheckRandomChance(0.25f))
	{
		Weapon* drop = nullptr;
		
		float chance = GetRandomFloatZeroToOne();

		if (chance > 0.5f)
		{
			drop = new Weapon(EntityDefinition::GetDefinition("Shotgun"));
		}
		else if (chance > 0.2f)
		{
			drop = new Weapon(EntityDefinition::GetDefinition("Flamethrower"));
		}
		else
		{
			drop = new Weapon(EntityDefinition::GetDefinition("MissileLauncher"));
		}

		drop->SetPosition(GetCenterPosition());
		Game::GetWorld()->AddEntity(drop);
	}

	if (GetTeam() == ENTITY_TEAM_ENEMY)
	{
		Game::AddPointsToScore(m_definition->m_pointValue);
	}
}


//-----------------------------------------------------------------------------------------------
// OnCollision override for allowing behaviors to have custom collision behavior
//
void AIEntity::OnEntityCollision(Entity* other)
{
	AnimatedEntity::OnEntityCollision(other);
	m_behaviorComponent->OnEntityCollision(other);
}


//-----------------------------------------------------------------------------------------------
// For setting up the behavior
//
void AIEntity::OnSpawn()
{
	AnimatedEntity::OnSpawn();
	m_behaviorComponent->OnSpawn();
}


//-----------------------------------------------------------------------------------------------
// Sets the spawn event that this entity spawned from
//
void AIEntity::SetSpawnEvent(EntitySpawnEvent* spawnEvent)
{
	m_eventSpawnedFrom = spawnEvent;
}
