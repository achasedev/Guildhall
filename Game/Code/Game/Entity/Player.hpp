#pragma once
#include "Engine/Core/GameObject.hpp"

class OrbitCamera;

class Player : public GameObject
{
public:
	//-----Public Methods-----

	Player();
	~Player();

	void ProcessInput();
	virtual void Update(float deltaTime) override;


private:
	//-----Private Methods-----

	void UpdateCameraOnInput();
	void UpdatePositionOnInput();

	void UpdateHeightOnMap();


private:
	//-----Private Data-----

	Vector3 m_position;
	OrbitCamera* m_camera;

};
