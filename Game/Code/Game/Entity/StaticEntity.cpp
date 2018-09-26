#include "Game/Entity/StaticEntity.hpp"

StaticEntity::StaticEntity()
	: Entity(ENTITY_TYPE_STATIC)
{
	// Make it unmovable from collisions
	m_mass = REALLY_BIG_MASS;
}

StaticEntity::~StaticEntity()
{
}

void StaticEntity::Update()
{
	Entity::Update();
}

void StaticEntity::OnCollision(Entity* other)
{
	Entity::OnCollision(other);
}

