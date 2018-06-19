/************************************************************************/
/* File: Player.hpp
/* Author: Andrew Chase
/* Date: June 5th, 2018
/* Description: Class to represent a player-controlled entity
/************************************************************************/
#pragma once
#include "Game/Entity/Tank.hpp"

class Camera;
class Stopwatch;
class OrbitCamera;

class Player : public Tank
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


private:
	//-----Private Data-----

	Stopwatch* m_stopwatch;
	OrbitCamera* m_camera;

	static const Vector3	CAMERA_TARGET_OFFSET;
	static const float		CAMERA_ROTATION_SPEED;

};
