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
	void UpdatePositionOnInput();

	void ToggleEjected();
	void SetEjected(bool newState);
	bool IsEjected() const;


private:
	//-----Private Data-----
	
	float	m_offsetDistance;
	Vector3 m_offsetDirection;

	bool						m_cameraEjected = false;

	static constexpr float		CAMERA_ROTATION_SPEED = 45.f;
	static constexpr float		CAMERA_TRANSLATION_SPEED = 10.f;

};
