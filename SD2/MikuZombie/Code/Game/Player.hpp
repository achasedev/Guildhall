/************************************************************************/
/* File: Player.hpp
/* Author: Andrew Chase
/* Date: May 9th, 2018
/* Description: Class to represent a first-person player
/************************************************************************/
#pragma once
#include "Engine/Core/GameObject.hpp"

class Camera;

class Player : public GameObject
{
public:
	//-----Public Methods-----

	Player();

	void ProcessInput(float deltaTime);
	virtual void Update(float deltaTime) override;


private:
	//-----Private Methods-----



private:
	//-----Private Data-----

	// Static constants
	static const float PLAYER_LOOK_SENSITIVITY;
	static const float PLAYER_MOVE_SPEED;

};
