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

	void TurnTowardsTarget(const Vector3& target);


private:
	//-----Private Data-----

	Cannon* m_cannon;

	static const float TURRET_ROTATION_SPEED;

};
