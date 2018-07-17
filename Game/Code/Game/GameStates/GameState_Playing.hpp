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
#include "Engine/Rendering/Animation/Animator.hpp"

class Camera;
class GameObject;
class RenderScene;
class Renderable;
class AnimationClip;
class Skeleton;

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

	Renderable* m_lilithModel = nullptr;
	Skeleton* m_lilithSkeleton = nullptr;
	AnimationClip* m_lilithAnimation = nullptr;

	Renderable* m_mayaModel = nullptr;
	Skeleton* m_mayaSkeleton = nullptr;
	AnimationClip* m_mayaAnimation = nullptr;

	Renderable* m_gageModel = nullptr;
	Skeleton* m_gageSkeleton = nullptr;
	AnimationClip* m_gageAnimation = nullptr;

	Renderable* m_chanModel			= nullptr;
	Skeleton* m_chanSkeleton	= nullptr;
	AnimationClip* m_chanIdle		= nullptr;
	AnimationClip* m_chanRun		= nullptr;
	bool m_isRunning = false;

	float m_timeScale = 1.0f;

	Animator m_animator;

	bool m_renderModel = true;
	bool m_renderSkeleton = false;
	bool m_renderAnimation = false;
	static constexpr float CAMERA_ROTATION_SPEED = 45.f;
	static constexpr float CAMERA_TRANSLATION_SPEED = 10.f;

};
