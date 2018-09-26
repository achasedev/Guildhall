#pragma once
#include "Game/Entity/DynamicEntity.hpp"

class TestBox : public DynamicEntity
{
public:
	//-----Public Methods-----

	TestBox();

	virtual void Update() override;

	// Events
	virtual void OnCollision(Entity* other) override;
	virtual void OnDamageTaken(int damageAmount) override;
	virtual void OnDeath() override;
	virtual void OnSpawn() override;


private:
	//-----Private Data-----

};
