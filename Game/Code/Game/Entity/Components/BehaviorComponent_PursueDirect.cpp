/************************************************************************/
/* File: BehaviorComponent_PursueDirect.cpp
/* Author: Andrew Chase
/* Date: October 15th 2018
/* Description: Implementation of the PursueDirect behavior
/************************************************************************/
#include "Game/Framework/Game.hpp"
#include "Game/Entity/Player.hpp"
#include "Game/Framework/Game.hpp"
#include "Game/Framework/World.hpp"
#include "Game/Entity/Components/BehaviorComponent_PursueDirect.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Time/Clock.hpp"
#include "Engine/Core/Utility/ErrorWarningAssert.hpp"


//-----------------------------------------------------------------------------------------------
// Update
//
void BehaviorComponent_PursueDirect::Update()
{
	BehaviorComponent::Update();

	if (m_closestPlayer != nullptr)
	{
		MoveToClosestPlayer();
	}
}


//-----------------------------------------------------------------------------------------------
// Makes a copy of this behavior and returns it
//
BehaviorComponent* BehaviorComponent_PursueDirect::Clone() const
{
	ASSERT_OR_DIE(m_owningEntity == nullptr, "Error: Behavior clone had non-null base members on prototype");
	return new BehaviorComponent_PursueDirect(*this);
}
