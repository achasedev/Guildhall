/************************************************************************/
/* File: BehaviorComponent_Bomber.hpp
/* Author: Andrew Chase
/* Date: February 23rd 2019
/* Description: Behavior where the entity flies above players and drops bombs
/************************************************************************/
#pragma once
#include "Game/Entity/Components/BehaviorComponent.hpp"
#include "Engine/Core/Time/Stopwatch.hpp"


class Player;

class BehaviorComponent_Bomber : public BehaviorComponent
{
public:
	//-----Public Methods-----

	BehaviorComponent_Bomber(const EntityDefinition* projectileDefinition);
	~BehaviorComponent_Bomber() {}

	virtual void				Initialize(AIEntity* owningEntity) override;
	virtual void				Update() override;
	virtual BehaviorComponent*	Clone() const override;
	virtual void				OnSpawn() override;


private:
	//-----Private Methods-----

	void FindNewTargetPosition();


private:
	//-----Private Data-----


	float m_moveSpeed = 50.f;
	const EntityDefinition* m_projectileDefinition = nullptr;
	Stopwatch m_bombTimer;
	float m_bombCooldown = 1.0f;

	Vector3 m_targetPosition;
	Vector3 m_movingDirection;


	static constexpr float TARGET_MOVING_THRESHOLD = 20.f;
	static constexpr float TARGET_HEIGHT_OFF_GROUND = 20.f;
	static constexpr float MIN_DISTANCE_TO_NEW_TARGET = 50.f;
	static constexpr float MIN_DISTANCE_TO_NEW_TARGET_SQUARED = MIN_DISTANCE_TO_NEW_TARGET * MIN_DISTANCE_TO_NEW_TARGET;

};
