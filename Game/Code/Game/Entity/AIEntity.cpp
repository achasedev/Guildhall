#include "Game/Entity/AIEntity.hpp"
#include "Game/Entity/Components/BehaviorComponent_Wander.hpp"

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
