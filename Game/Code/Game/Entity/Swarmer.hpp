#pragma once
#include "Game/Entity/GameEntity.hpp"

class Swarmer : public GameEntity
{
public:
	//-----Public Methods-----

	Swarmer(const Vector3& position, unsigned int team);
	~Swarmer();

	virtual void Update(float deltaTime) override;

	virtual void OnCollisionWithEntity(GameEntity* other) override;


private:
	//-----Private Methods-----

	void RotateTowardsTarget();

	bool IsAlmostFacingTarget();
	void MoveForward();


private:
	//-----Private Data-----

	Vector3 m_target;

	static const float SWARMER_ROTATION_SPEED;
	static const float SWARMER_TRANSLATION_SPEED;

	static const float MIN_DOT_TO_MOVE_FORWARD;
	static const float MIN_DOT_TO_SHOOT;

};
