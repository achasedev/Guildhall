/************************************************************************/
/* File: GameCamera.hpp
/* Author: Andrew Chase
/* Date: September 28th 2018
/* Description: Class to represent the single game camera
/************************************************************************/
#pragma once
#include "Engine/Rendering/Core/Camera.hpp"

class GameCamera : public Camera
{
public:
	//-----Public Methods-----
	
	GameCamera();
	~GameCamera();

	void UpdatePositionBasedOnPlayers();


private:
	//-----Private Data-----
	
	float	m_offsetDistance;
	Vector3 m_offsetDirection;

};
