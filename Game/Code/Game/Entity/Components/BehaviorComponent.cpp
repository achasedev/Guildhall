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
// 	// Update our distance map
// 	World* world = Game::GetWorld();
// 	m_navigationMap->SolveMapUpToDistance((float)world->GetDimensions().x); // Will need to provide a cost map here from world
}

