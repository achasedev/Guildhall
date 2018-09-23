#pragma once
#include "Game/Entity/Entity.hpp"

class StaticEntity : public Entity
{
public:
	//-----Public Methods-----

	StaticEntity();
	~StaticEntity();

	// Core
	virtual void Update() override;

	// Collision
	virtual void OnCollision(Entity* other) override;


private:
	//-----Private Data-----


};
