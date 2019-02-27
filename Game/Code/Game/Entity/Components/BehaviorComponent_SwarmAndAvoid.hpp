/************************************************************************/
/* File: BehaviorComponent_SwarmAndAvoid.hpp
/* Author: Andrew Chase
/* Date: February 23rd 2019
/* Description: Behavior where the entity moves towards the player,
/*				but also takes care to not run into statics or eachother
/************************************************************************/
#pragma once
#include "Game/Entity/Components/BehaviorComponent.hpp"

class Player;

class BehaviorComponent_SwarmAndAvoid : public BehaviorComponent
{
	friend class EntityDefinition;

public:
	//-----Public Methods-----

	BehaviorComponent_SwarmAndAvoid() {}
	~BehaviorComponent_SwarmAndAvoid() {}

	virtual void				Update() override;
	virtual BehaviorComponent*	Clone() const override;


private:
	//-----Private Data-----

	float m_elbowRoom = 10.f;

};
