/************************************************************************/
/* File: BehaviorComponent.cpp
/* Author: Andrew Chase
/* Date: October 15th 2018
/* Description: Implementation of the BehaviorComponent base class
/************************************************************************/
#include "Game/Entity/Player.hpp"
#include "Game/Framework/Game.hpp"
#include "Game/Framework/World.hpp"
#include "Game/Entity/Components/BehaviorComponent.hpp"
#include "Engine/Core/Utility/HeatMap.hpp"


//-----------------------------------------------------------------------------------------------
// Constructor
//
BehaviorComponent::BehaviorComponent()
{
}


//-----------------------------------------------------------------------------------------------
// Destructor
//
BehaviorComponent::~BehaviorComponent()
{
}


//-----------------------------------------------------------------------------------------------
// Initialize
//
void BehaviorComponent::Initialize(AnimatedEntity* owningEntity)
{
	m_owningEntity = owningEntity;
	m_closestPlayer = GetClosestPlayer();
}


//-----------------------------------------------------------------------------------------------
// Update
//
void BehaviorComponent::Update()
{
	m_closestPlayer = GetClosestPlayer();
}


//-----------------------------------------------------------------------------------------------
// Callback for handling entity behavior on collisions
//
void BehaviorComponent::OnEntityCollision(Entity* other)
{
	UNUSED(other);
}


//-----------------------------------------------------------------------------------------------
// Returns the closest player to the entity with this component
//
Player* BehaviorComponent::GetClosestPlayer() const
{
	Player** players = Game::GetPlayers();

	Vector3 closestPlayerPosition;
	float minDistance = 10000.f;
	Player* closestPlayer = nullptr;

	Vector3 currentPosition = m_owningEntity->GetPosition();

	for (int i = 0; i < MAX_PLAYERS; ++i)
	{
		if (Game::IsPlayerAlive(i))
		{
			Vector3 playerPosition = players[i]->GetPosition();
			float currDistance = (playerPosition - currentPosition).GetLengthSquared();

			if (closestPlayer == nullptr || currDistance < minDistance)
			{
				minDistance = currDistance;
				closestPlayerPosition = playerPosition;
				closestPlayer = players[i];
			}
		}
	}

	ASSERT_OR_DIE(closestPlayer != nullptr, "");
	return closestPlayer;
}


//-----------------------------------------------------------------------------------------------
// Returns the closest player the entity has line of sight on
//
Player* BehaviorComponent::GetClosestPlayerInSight() const
{
	return Game::GetPlayers()[0];
}


//-----------------------------------------------------------------------------------------------
// Moves the entity towards the closest player
//
void BehaviorComponent::MoveToClosestPlayer()
{
	Vector3 closestPlayerPosition = m_closestPlayer->GetPosition();
	Vector3 currPosition = m_owningEntity->GetPosition();

	Vector3 directionToPlayer = (closestPlayerPosition - currPosition).GetNormalized();

	m_owningEntity->Move(directionToPlayer.xz());
	m_owningEntity->Decelerate();
}
