/************************************************************************/
/* File: BehaviorComponent_PursueJump.cpp
/* Author: Andrew Chase
/* Date: October 15th 2018
/* Description: Implementation of the PursueJump behavior
/************************************************************************/
#include "Game/Entity/Player.hpp"
#include "Game/Framework/Game.hpp"
#include "Game/Framework/World.hpp"
#include "Game/Entity/Components/BehaviorComponent_PursueJump.hpp"
#include "Engine/Core/Utility/ErrorWarningAssert.hpp"


//-----------------------------------------------------------------------------------------------
// Constructor
//
BehaviorComponent_PursueJump::BehaviorComponent_PursueJump()
{
}


//-----------------------------------------------------------------------------------------------
// Update
//
void BehaviorComponent_PursueJump::Update()
{
	BehaviorComponent::Update();

	if (m_closestPlayer != nullptr)
	{
		MoveToClosestPlayer();
		m_owningEntity->Jump(); // Jump constantly
	}
}


//-----------------------------------------------------------------------------------------------
// Clones this behavior and returns it
//
BehaviorComponent* BehaviorComponent_PursueJump::Clone() const
{
	ASSERT_OR_DIE(m_owningEntity == nullptr, "Error: Behavior clone had non-null base members on prototype");
	return new BehaviorComponent_PursueJump();
}
