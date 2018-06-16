#pragma once
#include "Engine/Core/GameObject.hpp"

class Turret;

class Tank : public GameObject
{
public:
	//-----Public Methods-----

	Tank();
	~Tank();

	virtual void Update(float deltaTime) override;

	void SetTarget(bool hasTarget, const Vector3& target = Vector3::ZERO);


private:
	//-----Private Methods-----

	void UpdateHeightOnMap();
	void UpdateOrientationWithNormal();


protected:
	//-----Protected Data-----

	Turret* m_turret;

	bool m_hasTarget = false;
	bool m_lookAtTarget = true;
	Vector3 m_target;

	static const float TANK_ROTATION_SPEED;
	static const float TANK_TRANSLATION_SPEED;

};