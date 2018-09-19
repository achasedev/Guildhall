#include "Game/Entity/DynamicEntity.hpp"
#include "Game/Framework/Game.hpp"

#define GRAVITY_MAGNITUDE (9.81);

DynamicEntity::DynamicEntity()
	: Entity()
{
}

DynamicEntity::~DynamicEntity()
{

}

void DynamicEntity::Update()
{
	Entity::Update();

	// Apply all the physics
	ApplyPhysicsStep();
}

void DynamicEntity::ApplyForce(const Vector3& force)
{
	m_force += force;
}

void DynamicEntity::ApplyPhysicsStep()
{
	if (m_affectedByGravity)
	{
		m_force += Vector3::DIRECTION_DOWN * m_mass * GRAVITY_MAGNITUDE;
	}

	// Apply force
	m_acceleration += (m_force * m_inverseMass);

	// Apply acceleration
	float deltaTime = Game::GetDeltaTime();

	m_velocity = (m_acceleration * deltaTime);

	// Apply velocity
	m_position += (m_velocity * deltaTime);

	// Zero out the force
	m_force = Vector3::ZERO;
}
