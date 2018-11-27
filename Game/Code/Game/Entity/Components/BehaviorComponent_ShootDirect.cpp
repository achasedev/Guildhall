/************************************************************************/
/* File: BehaviorComponent_ShootDirect.cpp
/* Author: Andrew Chase
/* Date: November 26th 2018
/* Description: Implementation of the shoot direct class
/************************************************************************/
#include "Game/Entity/Weapon.hpp"
#include "Game/Entity/AnimatedEntity.hpp"
#include "Game/Entity/Components/BehaviorComponent_ShootDirect.hpp"


//-----------------------------------------------------------------------------------------------
// Constructor
//
BehaviorComponent_ShootDirect::BehaviorComponent_ShootDirect(const EntityDefinition* weaponDef)
	: m_weaponDefinition(weaponDef)
{
}


//-----------------------------------------------------------------------------------------------
// Creates the weapon and equips it to the entity
//
void BehaviorComponent_ShootDirect::Initialize(AnimatedEntity* owningEntity)
{
	BehaviorComponent::Initialize(owningEntity);
	m_weapon = new Weapon(m_weaponDefinition);
	m_weapon->OnEquip(m_owningEntity);
	m_weapon->SetHasInfiniteAmmo(true);
}


//-----------------------------------------------------------------------------------------------
// Update
//
void BehaviorComponent_ShootDirect::Update()
{
	BehaviorComponent::Update();

	if (m_closestPlayer != nullptr)
	{
		MoveToClosestPlayer();

		if (GetDistanceToClosestPlayer() <= m_shootRange)
		{
			m_weapon->Shoot();
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Clone
//
BehaviorComponent* BehaviorComponent_ShootDirect::Clone() const
{
	BehaviorComponent_ShootDirect* shootBehavior = new BehaviorComponent_ShootDirect(m_weaponDefinition);
	shootBehavior->m_shootRange = m_shootRange;

	return shootBehavior;
}
