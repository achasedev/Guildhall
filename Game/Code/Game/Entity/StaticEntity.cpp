#include "Game/Entity/StaticEntity.hpp"

StaticEntity::StaticEntity()
	: Entity(ENTITY_TYPE_STATIC)
{
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

