/************************************************************************/
/* File: GameState_Playing.hpp
/* Author: Andrew Chase
/* Date: May 21st, 2018
/* Description: Class to represent the state when gameplay is active
/************************************************************************/
#pragma once
#include <string>
#include <vector>
#include "Game/GameStates/GameState.hpp"

class Camera;
class GameObject;
class RenderScene;
class Renderable;
class AnimationClip;
class SkeletonBase;

class GameState_Playing : public GameState
{
public:
	//-----Public Methods-----

	GameState_Playing();
	~GameState_Playing();

	virtual void ProcessInput() override;
	virtual void Update() override;
	virtual void Render() const override;

	virtual void Enter() override;
	virtual void Leave() override;


private:
	//-----Private Methods-----

	void UpdateCameraOnInput();
	void CheckRenderInput();

	
private:
	//-----Private Data-----

	Camera* m_gameCamera;
	std::vector<GameObject*> m_gameObjects;

	Renderable* m_modelRenderable;
	const SkeletonBase* m_skeleton;
	AnimationClip* m_clip = nullptr;

	bool m_renderModel = true;
	bool m_renderSkeleton = true;



	static constexpr float CAMERA_ROTATION_SPEED = 45.f;
	static constexpr float CAMERA_TRANSLATION_SPEED = 5.f;

};
