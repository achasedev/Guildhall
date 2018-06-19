/************************************************************************/
/* File: NPCTank.hpp
/* Author: Andrew Chase
/* Date: June 18th, 2018
/* Description: Class to represent a single AI controlled tank
/************************************************************************/
#pragma once
#include "Game/Entity/Tank.hpp"

class NPCTank : public Tank
{
public:
	//-----Public Methods-----

	NPCTank(unsigned int team);

	virtual void Update(float deltaTime) override;


private:
	//-----Private Methods-----

	void RotateTowardsTarget();

	bool IsAlmostFacingTarget();
	void MoveForward();

	bool CanShootAtTarget();


private:
	//-----Private Data-----

	static const float MIN_DOT_TO_MOVE_FORWARD;
	static const float MIN_DOT_TO_SHOOT;

};
