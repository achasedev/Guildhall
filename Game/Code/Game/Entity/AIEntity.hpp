/************************************************************************/
/* File: AIEntity.hpp
/* Author: Andrew Chase
/* Date: October 2nd, 2018
/* Description: Class to represent an AI controlled entity in the game
/************************************************************************/
#pragma once
#include "Game/Entity/AnimatedEntity.hpp"

class SpawnPoint;
class BehaviorComponent;

class AIEntity : public AnimatedEntity
{
public:
	//-----Public Methods-----
	
	AIEntity(const EntityDefinition* definition);

	virtual void Update() override;
	virtual void OnDeath() override;

	void SetSpawnPoint(SpawnPoint* spawnPoint);


private:
	//-----Private Data-----
	
	SpawnPoint*			m_spawnPoint = nullptr;
	BehaviorComponent*	m_behaviorComponent = nullptr;

};
