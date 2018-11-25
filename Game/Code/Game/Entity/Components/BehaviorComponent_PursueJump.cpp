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
// Initializes the component by placing the jump sensor just outside the entity's collision boundary
//
void BehaviorComponent_PursueJump::Initialize(AnimatedEntity* owningEntity)
{
	BehaviorComponent::Initialize(owningEntity);
	m_jumpSensorDistance = m_owningEntity->GetDimensions().z * 0.5f + SENSOR_OFFSET_DISTANCE;
}


//-----------------------------------------------------------------------------------------------
// Update
//
void BehaviorComponent_PursueJump::Update()
{
	UNIMPLEMENTED();
}


//-----------------------------------------------------------------------------------------------
// Clones this behavior and returns it
//
BehaviorComponent* BehaviorComponent_PursueJump::Clone() const
{
	ASSERT_OR_DIE(m_owningEntity == nullptr, "Error: Behavior clone had non-null base members on prototype");
	return new BehaviorComponent_PursueJump();
}
