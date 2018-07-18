/************************************************************************/
/* File: GameState_Playing.hpp
/* Author: Andrew Chase
/* Date: July 17th, 2018
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
class Matrix44;

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

	void LoadGage();
	void LoadMaya();
	void LoadUnityChan();

	void UpdateGage();
	void UpdateMaya();
	void UpdateUnityChan();

	void RenderGageAnim() const;
	void RenderMayaAnim() const;
	void RenderUnityChanAnim() const;

	
private:
	//-----Private Data-----

	Camera* m_gameCamera;
	std::vector<GameObject*> m_gameObjects;

	// Maya
	Renderable*		m_mayaModel			= nullptr;
	Skeleton*		m_mayaSkeleton		= nullptr;
	AnimationClip*	m_mayaAnimation		= nullptr;

	// Gage
	Renderable*		m_gageModel			= nullptr;
	Skeleton*		m_gageSkeleton		= nullptr;
	AnimationClip*	m_gageAnimation		= nullptr;

	// Unity Chan
	mutable Animator	m_chanAnimator;
	Skeleton*			m_chanSkeleton		= nullptr;
	Renderable*			m_chanModel			= nullptr;
	AnimationClip*		m_chanIdle			= nullptr;
	AnimationClip*		m_chanRun			= nullptr;
	bool				m_isRunning			= false;

	bool m_renderModels		= true;
	bool m_renderSkeletons	= false;
	bool m_renderAnimations	= false;
	static constexpr float CAMERA_ROTATION_SPEED = 45.f;
	static constexpr float CAMERA_TRANSLATION_SPEED = 10.f;

};
