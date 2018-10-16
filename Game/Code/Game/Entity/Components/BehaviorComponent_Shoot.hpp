/************************************************************************/
/* File: BehaviorComponent_Shoot.hpp
/* Author: Andrew Chase
/* Date: October 16th 2018
/* Description: Behavior where the entity shoots at a player
/************************************************************************/
#pragma once
#include "Game/Entity/Components/BehaviorComponent.hpp"
#include "Engine/Core/Time/Stopwatch.hpp"

class EntityDefinition;

class BehaviorComponent_Shoot : public BehaviorComponent
{
public:
	//-----Public Methods-----
	
	BehaviorComponent_Shoot(const EntityDefinition* projectileDef, float fireRate);

	virtual void Update() override;
	virtual BehaviorComponent* Clone() const override;


public:
	//-----Public Data-----
	

private:
	//-----Private Methods-----
	
	void Shoot();

	
private:
	//-----Private Data-----
	
	Stopwatch m_shootTimer;
	float m_shootInterval;
	float m_fireRate;
	const EntityDefinition* m_projectileDefinition = nullptr;
	
};
