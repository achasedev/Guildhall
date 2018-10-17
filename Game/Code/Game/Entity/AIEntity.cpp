#include "Game/Entity/AIEntity.hpp"
#include "Game/Entity/Components/BehaviorComponent_PursuePath.hpp"
#include "Game/Framework/Game.hpp"
#include "Game/Framework/World.hpp"

AIEntity::AIEntity(const EntityDefinition* definition)
	: MovingEntity(definition)
{
	// Initialize the Behavior Component
	m_behaviorComponent = definition->CloneBehaviorPrototype(0);
	m_behaviorComponent->Initialize(this);
}

void AIEntity::Update()
{
	MovingEntity::Update();
	m_behaviorComponent->Update();
}

void AIEntity::OnDeath()
{
	MovingEntity::OnDeath();

	Game::GetWorld()->ParticalizeEntity(this);
}
