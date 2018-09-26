#pragma once
#include "Game/Entity/Entity.hpp"

#define REALLY_BIG_MASS (1000000)

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

protected:
	//-----Protected Methods-----


private:
	//-----Private Data-----


};
