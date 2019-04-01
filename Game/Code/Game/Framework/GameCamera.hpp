/************************************************************************/
/* File: GameCamera.hpp
/* Author: Andrew Chase
/* Date: February 9th 2019
/* Description: Class to represent a camera for player view (including noclip)
/************************************************************************/
#pragma once
#include "Engine/Rendering/Core/Camera.hpp"

enum eCameraMode
{
	CAMERA_MODE_ATTACHED_FIRST_PERSON,
	CAMERA_MODE_ATTACHED_THIRD_PERSON,
	CAMERA_MODE_ATTACHED_FIXED_ANGLE,
	CAMERA_MODE_DETACHED,
	NUM_CAMERA_MODES
};

class Entity;

class GameCamera : public Camera
{
public:
	//-----Public Methods-----

	void ProcessInput();
	void Update();

	void AttachToEntity(Entity* entity, eCameraMode mode);
	void Detach();


private:
	//-----Private Data-----

	Vector3 m_frameTranslation = Vector3::ZERO;
	Vector3 m_frameRotation = Vector3::ZERO;

	eCameraMode m_cameraMode = CAMERA_MODE_DETACHED;
	Entity* m_entityAttachedTo = nullptr;

};
