#pragma once
#include "Game/Entity/Bullet.hpp"

class ChargeBullet : public Bullet
{
public:

	ChargeBullet(const Vector3& position, const Quaternion& orientation, unsigned int team);
	~ChargeBullet();

	void OnCollisionWithEntity(GameEntity* other);


private:

	float m_aoeRadius = 10.f;
};