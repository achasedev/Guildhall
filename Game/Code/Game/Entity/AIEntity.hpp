/************************************************************************/
/* File: AIEntity.hpp
/* Author: Andrew Chase
/* Date: October 2nd, 2018
/* Description: Class to represent an AI controlled entity in the game
/************************************************************************/
#pragma once
#include "Game/Entity/AnimatedEntity.hpp"

class EntitySpawnEvent;
class BehaviorComponent;

class AIEntity : public AnimatedEntity
{
public:
	//-----Public Methods-----
	
	AIEntity(const EntityDefinition* definition);

	virtual void Update() override;
	virtual void OnDeath() override;
	virtual void OnEntityCollision(Entity* other) override;

	void SetSpawnEvent(EntitySpawnEvent* spawnEvent);


private:
	//-----Private Data-----
	
	EntitySpawnEvent*	m_eventSpawnedFrom = nullptr;
	BehaviorComponent*	m_behaviorComponent = nullptr;

};
