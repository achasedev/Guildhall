#include "Game/Entity/TestBox.hpp"
#include "Game/Framework/GameCommon.hpp"

TestBox::TestBox()
{
	m_collisionDef = CollisionDefinition_t(COLLISION_TYPE_BOX, 8.f, 8.f, 8.f);
	SetupVoxelTextures("Data/3DTextures/TestBox.qef");
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
