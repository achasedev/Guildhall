/************************************************************************/
/* File: GameCamera.hpp
/* Author: Andrew Chase
/* Date: February 9th 2019
/* Description: Class to represent a camera for player view (including noclip)
/************************************************************************/
#pragma once
#include "Engine/Rendering/Core/Camera.hpp"

class GameCamera : public Camera
{
public:
	//-----Public Methods-----

	void ProcessInput();
	void Update();


private:
	//-----Private Data-----

	Vector3 m_frameTranslation = Vector3::ZERO;
	Vector3 m_frameRotation = Vector3::ZERO;

};
