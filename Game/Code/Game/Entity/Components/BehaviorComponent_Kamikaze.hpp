/************************************************************************/
/* File: BehaviorComponent_Kamikaze.hpp
/* Author: Andrew Chase
/* Date: February 23rd 2019
/* Description: Behavior where the entity moves to a random player and then
				blows up
/************************************************************************/
#pragma once
#include "Game/Entity/Components/BehaviorComponent.hpp"
#include "Engine/Core/Time/Stopwatch.hpp"

enum eKamikazeState
{
	STATE_PURSUE,
	STATE_TICKING,
	STATE_EXPLODE
};

class Player;

class BehaviorComponent_Kamikaze : public BehaviorComponent
{
public:
	//-----Public Methods-----

	BehaviorComponent_Kamikaze() {}
	~BehaviorComponent_Kamikaze() {}

	virtual void				Update() override;
	virtual BehaviorComponent*	Clone() const override;
	virtual void				OnSpawn() override;


private:
	//-----Private Methods-----

	void UpdatePursue();
	void UpdateTicking();
	void UpdateExplode();


private:
	//-----Private Data-----

	eKamikazeState m_state = STATE_PURSUE;
	Player* m_targetedPlayer = nullptr;

	Stopwatch m_tickTimer;
	float m_normalizedTimeLastFrame = 0.f;
	int m_explosionDamage = 3;
	float m_explosionRadius = 8.f;
	float m_explosionImpulse = 50.f;

	static constexpr float DISTANCE_TO_EXPLODE = 5.f;
	static constexpr float TICK_DURATION = 2.f;

};
