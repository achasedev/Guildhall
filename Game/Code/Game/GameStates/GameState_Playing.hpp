/************************************************************************/
/* File: GameState_Playing.hpp
/* Author: Andrew Chase
/* Date: July 17th, 2018
/* Description: Class to represent the state when gameplay is active
/************************************************************************/
#pragma once
#include <string>
#include <vector>
#include "Game/GameStates/GameState.hpp"
#include "Engine/Rendering/Animation/Animator.hpp"


class GameState_Playing : public GameState
{
public:
	//-----Public Methods-----

	GameState_Playing();
	~GameState_Playing();

	virtual void ProcessInput() override;
	virtual void Update() override;
	virtual void Render() const override;

	virtual void Enter() override;
	virtual void Leave() override;


private:
	//-----Private Methods-----

	void UpdateCameraOnInput();

	
private:
	//-----Private Data-----

	static constexpr float CAMERA_ROTATION_SPEED = 45.f;
	static constexpr float CAMERA_TRANSLATION_SPEED = 10.f;

};
