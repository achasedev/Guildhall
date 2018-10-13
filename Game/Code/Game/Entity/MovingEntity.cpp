#include "Game/Framework/Game.hpp"
#include "Game/Entity/MovingEntity.hpp"
#include "Game/Entity/Components/PhysicsComponent.hpp"
#include "Engine/Math/MathUtils.hpp"

MovingEntity::MovingEntity(const EntityDefinition* definition)
	: Entity(definition)
{
}

MovingEntity::~MovingEntity()
{

}

void MovingEntity::Update()
{
	Entity::Update();
}

void MovingEntity::OnCollision(Entity* other)
{
	Entity::OnCollision(other);
}

void MovingEntity::OnDamageTaken(int damageAmount)
{
	Entity::OnDamageTaken(damageAmount);
}

void MovingEntity::OnDeath()
{
	Entity::OnDeath();
}

void MovingEntity::OnSpawn()
{
	Entity::OnSpawn();
}

void MovingEntity::Move(const Vector2& direction)
{
	float currLateralSpeed = m_physicsComponent->GetVelocity().xz().GetLength();
	float deltaTime = Game::GetDeltaTime();

	Vector2 maxLateralVelocity = (m_physicsComponent->GetVelocity().xz() + (m_definition->m_maxMoveAcceleration * deltaTime) * direction);
	float maxLateralSpeed = maxLateralVelocity.NormalizeAndGetLength();

	maxLateralSpeed = (currLateralSpeed > m_definition->m_maxMoveSpeed ? ClampFloat(maxLateralSpeed, 0.f, currLateralSpeed) : ClampFloat(maxLateralSpeed, 0.f, m_definition->m_maxMoveSpeed));
	maxLateralVelocity *= maxLateralSpeed;

	Vector3 inputVelocityResult = Vector3(maxLateralVelocity.x, m_physicsComponent->GetVelocity().y, maxLateralVelocity.y) - m_physicsComponent->GetVelocity();
	Vector3 acceleration = inputVelocityResult / deltaTime;
	Vector3 force = acceleration * m_mass;

	m_physicsComponent->AddForce(force);

	// Reorient the entity
	//m_orientation = direction.GetOrientationDegrees();
}

void MovingEntity::Jump()
{
	m_physicsComponent->AddImpulse(Vector3::DIRECTION_UP * m_definition->m_jumpImpulse);
}

void MovingEntity::Decelerate()
{
	float deltaTime = Game::GetDeltaTime();
	float currSpeed = m_physicsComponent->GetVelocity().xz().GetLength();
	float amountCanBeDecreased = currSpeed;

	if (amountCanBeDecreased > 0.f)
	{
		Vector2 direction = -1.0f * m_physicsComponent->GetVelocity().xz().GetNormalized();

		float decelMag = amountCanBeDecreased / deltaTime;
		decelMag = ClampFloat(decelMag, 0.f, m_definition->m_maxMoveDeceleration);

		float forceMag = decelMag * m_mass;

		direction *= forceMag;
		Vector3 finalForce = Vector3(direction.x, 0.f, direction.y);

		m_physicsComponent->AddForce(finalForce);
	}
}

