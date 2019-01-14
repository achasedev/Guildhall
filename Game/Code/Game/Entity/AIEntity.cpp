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
#include "Game/Framework/SpawnPoint.hpp"
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

	m_spawnPoint->StopTrackingEntity(this);
	Game::GetWorld()->ParticalizeEntity(this);

	if (CheckRandomChance(0.25f))
	{
		Weapon* drop = nullptr;

		float roll = GetRandomFloatZeroToOne();

		if (roll > 0.9f)
		{
			drop = new Weapon(EntityDefinition::GetDefinition("MissileLauncher"));
		}
		else if (roll > 0.6f)
		{
			drop = new Weapon(EntityDefinition::GetDefinition("Flamethrower"));
		}
		else
		{
			drop = new Weapon(EntityDefinition::GetDefinition("Shotgun"));
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
// Sets the spawn point of this entity to the one given
//
void AIEntity::SetSpawnPoint(SpawnPoint* spawnPoint)
{
	m_spawnPoint = spawnPoint;
}
