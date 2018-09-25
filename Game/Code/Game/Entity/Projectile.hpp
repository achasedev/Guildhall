#pragma once
#include "Game/Entity/DynamicEntity.hpp"

class Stopwatch;

class Projectile : public DynamicEntity
{
public:
	//-----Public Methods-----

	Projectile();

	virtual void Update() override;

	// Events
	virtual void OnCollision(Entity* other) override;
	virtual void OnDamageTaken(int damageAmount) override;
	virtual void OnDeath() override;
	virtual void OnSpawn() override;


private:
	//-----Private Data-----

	float		m_speed = DEFAULT_SPEED;
	float		m_lifetime = DEFAULT_LIFETIME;
	Stopwatch*	m_stopwatch = nullptr;

	static constexpr float DEFAULT_LIFETIME = 1.0f;
	static constexpr float DEFAULT_SPEED = 100.f;

};
