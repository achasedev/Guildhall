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


private:
	//-----Private Methods-----

	void UpdateHeightOnMap();
	void UpdateOrientationWithNormal();


private:
	//-----Private Data-----

	Turret* m_turret;
};