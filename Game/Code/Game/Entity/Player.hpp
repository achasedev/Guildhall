#pragma once
#include "Engine/Core/GameObject.hpp"

class Camera;
class OrbitCamera;

class Player : public GameObject
{
public:
	//-----Public Methods-----

	Player();
	~Player();

	void ProcessInput();
	virtual void Update(float deltaTime) override;

	Camera* GetCamera() const;


private:
	//-----Private Methods-----

	void UpdateCameraOnInput(float deltaTime);
	void UpdatePositionOnInput(float deltaTime);

	void UpdateHeightOnMap();


private:
	//-----Private Data-----

	OrbitCamera* m_camera;

	static const float CAMERA_ROTATION_SPEED;
	static const float CAMERA_TRANSLATION_SPEED;
};
