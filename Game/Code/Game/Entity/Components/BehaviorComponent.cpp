#include "Game/Framework/Game.hpp"
#include "Game/Framework/World.hpp"
#include "Game/Entity/Components/BehaviorComponent.hpp"
#include "Engine/Core/Utility/HeatMap.hpp"

BehaviorComponent::BehaviorComponent()
{
}

BehaviorComponent::~BehaviorComponent()
{
	if (m_navigationMap != nullptr)
	{
		delete m_navigationMap;
		m_navigationMap = nullptr;
	}
}

void BehaviorComponent::Initialize(MovingEntity* owningEntity)
{
	m_owningEntity = owningEntity;

	World* world = Game::GetWorld();
	m_navigationMap = new HeatMap(world->GetDimensions().xz(), -1.f);
}

void BehaviorComponent::Update()
{
// 	// Update our distance map
// 	World* world = Game::GetWorld();
// 	m_navigationMap->SolveMapUpToDistance((float)world->GetDimensions().x); // Will need to provide a cost map here from world
}

