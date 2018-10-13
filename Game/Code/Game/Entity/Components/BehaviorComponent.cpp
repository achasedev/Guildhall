#include "Game/Framework/Game.hpp"
#include "Game/Framework/World.hpp"
#include "Game/Entity/Components/BehaviorComponent.hpp"
#include "Engine/Core/Utility/HeatMap.hpp"

BehaviorComponent::BehaviorComponent()
{
}

BehaviorComponent::~BehaviorComponent()
{
}

void BehaviorComponent::Initialize(MovingEntity* owningEntity)
{
	m_owningEntity = owningEntity;
}

void BehaviorComponent::Update()
{
}

