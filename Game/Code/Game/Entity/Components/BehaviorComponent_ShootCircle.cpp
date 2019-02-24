/************************************************************************/
/* File: BehaviorComponent_ShootCircle.cpp
/* Author: Andrew Chase
/* Date: November 26th 2018
/* Description: Implementation of the shoot circle class
/************************************************************************/
#include "Game/Entity/Weapon.hpp"
#include "Game/Entity/Player.hpp"
#include "Game/Entity/AIEntity.hpp"
#include "Game/Entity/Components/BehaviorComponent_ShootCircle.hpp"
#include "Engine/Math/MathUtils.hpp"

//-----------------------------------------------------------------------------------------------
// Constructor
//
BehaviorComponent_ShootCircle::BehaviorComponent_ShootCircle(const EntityDefinition* weaponDef)
	: m_weaponDefinition(weaponDef)
{
}


//-----------------------------------------------------------------------------------------------
// Creates the weapon and equips it to the entity
//
void BehaviorComponent_ShootCircle::Initialize(AIEntity* owningEntity)
{
	BehaviorComponent::Initialize(owningEntity);
	m_weapon = new Weapon(m_weaponDefinition);
	m_weapon->OnEquip(m_owningEntity);
	m_weapon->SetHasInfiniteAmmo(true);

	if (CheckRandomChance(0.5f))
	{
		m_goLeft = true;
	}
}


//-----------------------------------------------------------------------------------------------
// Update
//
void BehaviorComponent_ShootCircle::Update()
{
	BehaviorComponent::Update();

	if (m_closestPlayer == nullptr)
	{
		return;
	}

	float distance = GetDistanceToClosestPlayer();

	if (distance > m_shootRange)
	{
		MoveToClosestPlayer();
	}
	else
	{
		// Begin circling and shooting
		Vector2 directionToPlayer = (m_closestPlayer->GetPosition() - m_owningEntity->GetPosition()).xz().GetNormalized();
		Vector2 tangent = Vector2(-directionToPlayer.y, directionToPlayer.x);

		if (m_goLeft)
		{
			tangent *= -1.0f;
		}

		m_owningEntity->Move(tangent);
		m_owningEntity->SetOrientation(directionToPlayer.GetOrientationDegrees());
		m_weapon->Shoot();
	}

	if (GetDistanceToClosestPlayer() <= m_shootRange)
	{
		m_weapon->Shoot();
	}
}


//-----------------------------------------------------------------------------------------------
// Clone
//
BehaviorComponent* BehaviorComponent_ShootCircle::Clone() const
{
	BehaviorComponent_ShootCircle* shootBehavior = new BehaviorComponent_ShootCircle(m_weaponDefinition);
	shootBehavior->m_shootRange = m_shootRange;

	return shootBehavior;
}
