/************************************************************************/
/* File: GameCamera.hpp
/* Author: Andrew Chase
/* Date: September 28th 2018
/* Description: Class to represent the single game camera
/************************************************************************/
#pragma once
#include "Engine/Core/Time/Stopwatch.hpp"
#include "Engine/Rendering/Core/Camera.hpp"

enum eCameraState
{
	STATE_FIXED,
	STATE_FREE,
	STATE_FOLLOW,
	NUM_STATES
};

class GameCamera : public Camera
{
public:
	//-----Public Methods-----
	
	GameCamera();
	~GameCamera();

	void ProcessInput();
	void UpdateBasedOnState();

	void AddScreenShake(float addedScreenShakeMagnitude);


private:
	//-----Private Methods-----

	void UpdateFollow();
	void UpdateFree();
	void UpdateFixed();


private:
	//-----Private Data-----
	
	Vector3 m_frameTranslation = Vector3::ZERO;
	Vector3 m_frameRotation = Vector3::ZERO;

	float	m_offsetDistance;
	Vector3 m_offsetDirection;

	eCameraState				m_state = STATE_FIXED;

	Stopwatch					m_screenShakeInterval;

	static constexpr float		CAMERA_ROTATION_SPEED = 45.f;
	static constexpr float		CAMERA_TRANSLATION_SPEED = 10.f;

};
