#pragma once
#include "Engine/Core/GameObject.hpp"

class Cannon : public GameObject
{
public:
	//-----Public Methods-----

	Cannon(Transform& parent);
	~Cannon();

	virtual void Update(float deltaTime) override;

	Matrix44 GetFireTransform();

	void ElevateTowardsTarget(const Vector3& target);


private:
	//-----Private Data-----

	Transform m_muzzleTransform;

	static const float CANNON_ROTATION_SPEED;

};
