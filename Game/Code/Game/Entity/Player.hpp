/************************************************************************/
/* File: Player.hpp
/* Author: Andrew Chase
/* Date: June 5th, 2018
/* Description: Class to represent a player-controlled entity
/************************************************************************/
#pragma once
#include "Engine/Core/GameObject.hpp"

class Camera;
class Stopwatch;
class OrbitCamera;

class Player : public GameObject
{
public:
	//-----Public Methods-----

	Player();
	~Player();

	void			ProcessInput();
	virtual void	Update(float deltaTime) override;

	// Accessors
	Camera* GetCamera() const;


private:
	//-----Private Methods-----

	void UpdateCameraOnInput(float deltaTime);
	void UpdatePositionOnInput(float deltaTime);

	void UpdateHeightOnMap();
	void UpdateOrientationWithNormal();


private:
	//-----Private Data-----

	Stopwatch* m_stopwatch;
	OrbitCamera* m_camera;

	static const Vector3	CAMERA_TARGET_OFFSET;
	static const float		CAMERA_ROTATION_SPEED;
	static const float		PLAYER_ROTATION_SPEED;
	static const float		PLAYER_TRANSLATION_SPEED;
};
