#include "Game/Entity/TestBox.hpp"
#include "Game/Framework/GameCommon.hpp"

TestBox::TestBox()
{
	m_collisionDef = CollisionDefinition_t(COLLISION_SHAPE_BOX, COLLISION_RESPONSE_SHARE_CORRECTION, 4.f, 4.f, 8.f);
}

void TestBox::Update()
{
	DynamicEntity::Update();
}

void TestBox::OnCollision(Entity* other)
{
	UNUSED(other);
}

void TestBox::OnDamageTaken(int damageAmount)
{
	UNUSED(damageAmount);
}

void TestBox::OnDeath()
{
}

void TestBox::OnSpawn()
{
}
