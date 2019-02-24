/************************************************************************/
/* File: BehaviorComponent_ShootCircle.hpp
/* Author: Andrew Chase
/* Date: November 26th 2018
/* Description: Behavior that has the entity circle a player while
/*				shooting at them
/************************************************************************/
#pragma once
#include "Game/Entity/Components/BehaviorComponent.hpp"

class Weapon;
class EntityDefinition;

class BehaviorComponent_ShootCircle : public BehaviorComponent
{
	friend class EntityDefinition;

public:
	//-----Public Methods-----

	BehaviorComponent_ShootCircle(const EntityDefinition* weaponDef);

	virtual void Initialize(AIEntity* owningEntity) override;
	virtual void Update() override;
	virtual BehaviorComponent* Clone() const override;


private:
	//-----Private Data-----

	Weapon* m_weapon = nullptr;
	float m_shootRange = 50.f;
	const EntityDefinition* m_weaponDefinition = nullptr;

	bool m_goLeft = false;

};
