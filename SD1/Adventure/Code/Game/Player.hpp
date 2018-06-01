/************************************************************************/
/* File: Player.hpp
/* Author: Andrew Chase
/* Date: November 16th, 2017
/* Bugs: None
/* Description: Class to represent a player-controlled actor
/************************************************************************/
#pragma once
#include "Game/Actor.hpp"


class Player : public Actor
{
public:
	//-----Public Methods-----

	Player(const Vector2& position, float orientation, const ActorDefinition* definition, Map* entityMap);
	
	void Update(float deltaTime) override;
	void Render() const override;

	void Respawn();

private:
	//-----Private Methods-----

	bool UpdateOnControllerInput(float deltaTime);
	bool UpdateOnKeyboardInput(float deltaTime);

	void ShootArrow();
	void ThrustSpear();

private:
	//-----Private Data-----

	float m_attackCooldown;

	static constexpr float PLAYER_ATTACKS_PER_SECOND = 2.f;
};
