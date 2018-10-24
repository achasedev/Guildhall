/************************************************************************/
/* File: BehaviorComponent_PursueDirect.cpp
/* Author: Andrew Chase
/* Date: October 15th 2018
/* Description: Implementation of the PursueDirect behavior
/************************************************************************/
#include "Game/Entity/Player.hpp"
#include "Game/Framework/Game.hpp"
#include "Game/Entity/Components/BehaviorComponent_PursueDirect.hpp"
#include "Engine/Core/Utility/ErrorWarningAssert.hpp"


//-----------------------------------------------------------------------------------------------
// Update
//
void BehaviorComponent_PursueDirect::Update()
{
	BehaviorComponent::Update();

	Player** players = Game::GetPlayers();

	Vector3 closestPlayerPosition;
	float minDistance = 10000.f;
	bool playerFound = false;

	Vector3 currentPosition = m_owningEntity->GetEntityPosition();

	for (int i = 0; i < MAX_PLAYERS; ++i)
	{
		if (Game::IsPlayerAlive(i))
		{
			Vector3 playerPosition = players[i]->GetEntityPosition();
			float currDistance = (playerPosition - currentPosition).GetLengthSquared();

			if (!playerFound || currDistance < minDistance)
			{
				minDistance = currDistance;
				closestPlayerPosition = playerPosition;
				playerFound = true;
			}
		}
	}
	
	// Shouldn't happen, but to avoid unidentifiable behavior
	if (!playerFound)
	{
		return;
	}

	Vector3 directionToMove = (closestPlayerPosition - currentPosition).GetNormalized();
	m_owningEntity->Move(directionToMove.xz());
}


//-----------------------------------------------------------------------------------------------
// Makes a copy of this behavior and returns it
//
BehaviorComponent* BehaviorComponent_PursueDirect::Clone() const
{
	ASSERT_OR_DIE(m_owningEntity == nullptr, "Error: Behavior clone had non-null base members on prototype");
	return new BehaviorComponent_PursueDirect();
}
