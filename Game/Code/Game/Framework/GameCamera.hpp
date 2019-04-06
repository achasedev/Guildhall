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

	inline bool IsAttachedToEntity(const Entity* entity);
	inline eCameraMode GetCameraMode() const;


private:
	//-----Private Methods-----

	void ProcessInputDetached();
	void ProcessInputFixedAngle();
	void ProcessInputThirdPerson();

	void CheckIfEntityStillValid();
	void UpdateFirstPerson();
	void UpdateThirdPerson();
	void UpdateFixedAngle();
	void UpdateDetached();


private:
	//-----Private Data-----

	Vector3 m_frameTranslation = Vector3::ZERO;
	Vector3 m_frameRotation = Vector3::ZERO;

	Vector3 m_orbitSphericalRotation = Vector3::ZERO;

	eCameraMode m_cameraMode = CAMERA_MODE_DETACHED;
	Entity* m_entityAttachedTo = nullptr;
	float m_offsetMagnitude = (CAMERA_THIRD_PERSON_MAX_DISTANCE + CAMERA_THIRD_PERSON_MIN_DISTANCE) * 0.5f;

	// Statics
	static const Vector3 CAMERA_FIXED_ANGLE_DIRECTION;
	static constexpr float CAMERA_THIRD_PERSON_MIN_DISTANCE = 2.f;
	static constexpr float CAMERA_THIRD_PERSON_MAX_DISTANCE = 10.f;

};


//-----------------------------------------------------------------------------------------------
// Returns whether this camera is attached to the given entity, in any mode
//
inline bool GameCamera::IsAttachedToEntity(const Entity* entity)
{
	return (m_entityAttachedTo == entity);
}


//-----------------------------------------------------------------------------------------------
// Returns the current mode of the camera
//
inline eCameraMode GameCamera::GetCameraMode() const
{
	return m_cameraMode;
}
