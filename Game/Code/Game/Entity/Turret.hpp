#pragma once
#include "Engine/Core/GameObject.hpp"

class Cannon;

class Turret : public GameObject
{
public:
	//-----Public Methods-----

	Turret(Transform& parent);
	~Turret();

	virtual void Update(float deltaTime) override;


private:
	//-----Private Data-----

	Cannon* m_cannon;

};
