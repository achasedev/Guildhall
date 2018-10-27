/************************************************************************/
/* File: BehaviorComponent_Wander.cpp
/* Author: Andrew Chase
/* Date: October 9th, 2018
/* Description: Implementation of the PursuePath behavior
/************************************************************************/
#include "Game/Framework/Game.hpp"
#include "Game/Framework/World.hpp"
#include "Game/Entity/AnimatedEntity.hpp"
#include "Game/Entity/Components/BehaviorComponent_PursuePath.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Utility/HeatMap.hpp"
#include "Engine/Core/Utility/ErrorWarningAssert.hpp"


//-----------------------------------------------------------------------------------------------
// Constructor
//
BehaviorComponent_PursuePath::BehaviorComponent_PursuePath()
{
}


//-----------------------------------------------------------------------------------------------
// Destructor
//
BehaviorComponent_PursuePath::~BehaviorComponent_PursuePath()
{
}


//-----------------------------------------------------------------------------------------------
// Update
//
void BehaviorComponent_PursuePath::Update()
{ 
	BehaviorComponent::Update();

	// Move towards next position
	Vector3 nextPosition = Game::GetWorld()->GetNextPositionTowardsPlayer(m_owningEntity->GetPosition());
	Vector2 toNext = (nextPosition - m_owningEntity->GetPosition()).GetNormalized().xz();
	
	m_owningEntity->Move(toNext);
}


//-----------------------------------------------------------------------------------------------
// Makes a copy of the PursuePath behavior and returns it
//
BehaviorComponent* BehaviorComponent_PursuePath::Clone() const
{
	ASSERT_OR_DIE(m_owningEntity == nullptr, "Error: Behavior clone had non-null base members on prototype");
	BehaviorComponent* clone = new BehaviorComponent_PursuePath();

	return clone;
}
