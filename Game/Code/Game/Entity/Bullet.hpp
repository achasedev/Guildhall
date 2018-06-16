#pragma once
#include "Engine/Core/GameObject.hpp"

class Bullet : public GameObject
{
public:
	//-----Public Methods-----

	Bullet(const Vector3& position, const Quaternion& orientation);
	~Bullet();

	virtual void Update(float deltaTime) override;


private:
	//-----Private Data-----

	static const float BULLET_SPEED;

};
