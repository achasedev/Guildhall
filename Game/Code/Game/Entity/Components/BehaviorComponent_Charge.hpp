/************************************************************************/
/* File: BehaviorComponent_Charge.hpp
/* Author: Andrew Chase
/* Date: November 26th 2018
/* Description: Behavior for when an entity charges at a player
/************************************************************************/
#pragma once
#include "Game/Entity/Components/BehaviorComponent.hpp"
#include "Engine/Core/Time/Stopwatch.hpp"
#include "Engine/Math/Vector2.hpp"

enum eChargeState
{
	STATE_SEARCH,
	STATE_JUMP,
	STATE_CHARGE,
	STATE_KNOCKBACK,
	STATE_REST
};


class BehaviorComponent_Charge : public BehaviorComponent
{
	friend class EntityDefinition;

public:
	//-----Public Methods-----
	
	BehaviorComponent_Charge();
	~BehaviorComponent_Charge() {}

	virtual void				Update() override;
	virtual BehaviorComponent*	Clone() const override;

	virtual void				OnEntityCollision(Entity* other) override;


public:
	//-----Public Data-----
	

private:
	//-----Private Methods-----

	void ContinueCharge();


private:
	//-----Private Data-----
	
	eChargeState m_state;
	Stopwatch m_stateTimer;
	Vector2 m_chargeDirection;

	float	m_chargeDuration = 2.f;
	float	m_restDuration = 1.f;
	float	m_chargeSpeed = 100.f;
	float	m_knockbackMagnitude = 30.f;
	int		m_damageOnCharge = 2;

};
