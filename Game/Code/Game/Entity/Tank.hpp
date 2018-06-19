/************************************************************************/
/* File: Tank.hpp
/* Author: Andrew Chase
/* Date: June 18th, 2018
/* Description: Class to represent a tank in game (player or npc)
/************************************************************************/
#pragma once
#include "Engine/Core/GameObject.hpp"

class Turret;
class Stopwatch;

class Tank : public GameObject
{
public:
	//-----Public Methods-----

	Tank(unsigned int team);
	~Tank();

	virtual void Update(float deltaTime) override;

	// Mutators
	void SetTarget(bool hasTarget, const Vector3& target = Vector3::ZERO);

	// Producers
	void ShootCannon();


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
	float m_fireRate;	// Shells per second the tank can fire
	Stopwatch* m_stopwatch;

	unsigned int m_team;

	static const float TANK_ROTATION_SPEED;
	static const float TANK_TRANSLATION_SPEED;
	static const float TANK_DEFAULT_FIRERATE;

};
