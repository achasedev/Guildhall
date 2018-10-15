/************************************************************************/
/* File: BehaviorComponent_Wander.cpp
/* Author: Andrew Chase
/* Date: October 9th, 2018
/* Description: Implementation of the wander behavior
/************************************************************************/
#include "Game/Framework/Game.hpp"
#include "Game/Framework/World.hpp"
#include "Game/Entity/MovingEntity.hpp"
#include "Game/Entity/Components/BehaviorComponent_PursuePath.hpp"

#include "Engine/Core/Utility/HeatMap.hpp"
#include "Engine/Core/Utility/ErrorWarningAssert.hpp"
#include "Engine/Math/MathUtils.hpp"

#include "Engine/Core/Rgba.hpp"
#include "Engine/Rendering/DebugRendering/DebugRenderSystem.hpp"

BehaviorComponent_PursuePath::BehaviorComponent_PursuePath()
{
}

BehaviorComponent_PursuePath::~BehaviorComponent_PursuePath()
{
}

void BehaviorComponent_PursuePath::Update()
{ 
	BehaviorComponent::Update();

	// Move towards next position
	Vector3 nextPosition = Game::GetWorld()->GetNextPositionTowardsPlayer(m_owningEntity->GetEntityPosition());
	Vector2 toNext = (nextPosition - m_owningEntity->GetEntityPosition()).xz();
	
	m_owningEntity->Move(toNext);
}

BehaviorComponent* BehaviorComponent_PursuePath::Clone() const
{
	ASSERT_OR_DIE(m_owningEntity == nullptr, "Error: Behavior clone had non-null base members on prototype");
	BehaviorComponent* clone = new BehaviorComponent_PursuePath();

	return clone;
}
