/************************************************************************/
/* File: BehaviorComponent_SwarmAndAvoid.hpp
/* Author: Andrew Chase
/* Date: February 23rd 2018
/* Description: Behavior where the entity moves towards the player,
/*				but also takes care to not run into statics or eachother
/************************************************************************/
#pragma once
#include "Game/Entity/Components/BehaviorComponent.hpp"

class Player;

class BehaviorComponent_SwarmAndAvoid : public BehaviorComponent
{
public:
	//-----Public Methods-----

	BehaviorComponent_SwarmAndAvoid() {}
	~BehaviorComponent_SwarmAndAvoid() {}

	virtual void				Update() override;
	virtual BehaviorComponent*	Clone() const override;


private:
	//-----Private Data-----

	float m_elbowRoom = 9.f;
	float m_holeDetectionRange = 8.f;

};
