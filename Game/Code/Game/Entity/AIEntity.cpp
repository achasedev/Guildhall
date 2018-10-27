/************************************************************************/
/* File: AIEntity.cpp
/* Author: Andrew Chase
/* Date: October 20th 2018
/* Description: Implementation of the AIEntity class
/************************************************************************/
#include "Game/Framework/Game.hpp"
#include "Game/Entity/AIEntity.hpp"
#include "Game/Framework/World.hpp"
#include "Game/Framework/SpawnPoint.hpp"
#include "Game/Entity/Components/BehaviorComponent_PursuePath.hpp"


//-----------------------------------------------------------------------------------------------
// Constructor
//
AIEntity::AIEntity(const EntityDefinition* definition)
	: MovingEntity(definition)
{
	// Initialize the Behavior Component
	m_behaviorComponent = definition->CloneBehaviorPrototype(0);
	m_behaviorComponent->Initialize(this);
}


//-----------------------------------------------------------------------------------------------
// Update
//
void AIEntity::Update()
{
	MovingEntity::Update();
	m_behaviorComponent->Update();
}


//-----------------------------------------------------------------------------------------------
// Death event, removes itself from the spawner's list of entities it spawned. Also particalizes
//
void AIEntity::OnDeath()
{
	MovingEntity::OnDeath();

	m_spawnPoint->StopTrackingEntity(this);
	//Game::GetWorld()->ParticalizeEntity(this);
}

//-----------------------------------------------------------------------------------------------
// Sets the spawn point of this entity to the one given
//
void AIEntity::SetSpawnPoint(SpawnPoint* spawnPoint)
{
	m_spawnPoint = spawnPoint;
}
