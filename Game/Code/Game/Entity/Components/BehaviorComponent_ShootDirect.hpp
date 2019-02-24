/************************************************************************/
/* File: BehaviorComponent_Shoot.hpp
/* Author: Andrew Chase
/* Date: October 16th 2018
/* Description: Behavior where the entity shoots at a player while moving
/*				Directly at them
/************************************************************************/
#pragma once
#include "Game/Entity/Components/BehaviorComponent.hpp"
#include "Engine/Core/Time/Stopwatch.hpp"

class Weapon;
class EntityDefinition;

class BehaviorComponent_ShootDirect : public BehaviorComponent
{
	friend class EntityDefinition;

public:
	//-----Public Methods-----
	
	BehaviorComponent_ShootDirect(const EntityDefinition* weaponDef);

	virtual void Initialize(AIEntity* owningEntity) override;
	virtual void Update() override;
	virtual BehaviorComponent* Clone() const override;


private:
	//-----Private Data-----
	
	Weapon* m_weapon = nullptr;
	float m_shootRange = 50.f;
	const EntityDefinition* m_weaponDefinition = nullptr;
	
};
