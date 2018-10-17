/************************************************************************/
/* File: AIEntity.hpp
/* Author: Andrew Chase
/* Date: October 2nd, 2018
/* Description: Class to represent an AI controlled entity in the game
/************************************************************************/
#pragma once
#include "Game/Entity/MovingEntity.hpp"

class BehaviorComponent;

class AIEntity : public MovingEntity
{
public:
	//-----Public Methods-----
	
	AIEntity(const EntityDefinition* definition);

	virtual void Update() override;

	virtual void OnDeath() override;


private:
	//-----Private Data-----
	
	// AI controller stuff here
	BehaviorComponent* m_behaviorComponent = nullptr;

};
