/************************************************************************/
/* File: GameState_Playing.cpp
/* Author: Andrew Chase
/* Date: May 21st, 2018
/* Description: Implementation of the GameState_Playing class
/************************************************************************/
#include "Game/Framework/App.hpp"
#include "Game/Framework/Game.hpp"
#include "Game/Framework/GameCommon.hpp"
#include "Game/GameStates/GameState_Playing.hpp"

#include "Engine/Assets/AssetDB.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Time/Clock.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Assets/AssimpLoader.hpp"
#include "Engine/Rendering/Core/Camera.hpp"
#include "Engine/Rendering/Core/Renderer.hpp"
#include "Engine/Rendering/Core/Renderable.hpp"
#include "Engine/Rendering/Meshes/MeshGroup.hpp"
#include "Engine/Rendering/Core/RenderScene.hpp"
#include "Engine/Rendering/Resources/Sampler.hpp"
#include "Engine/Rendering/Materials/Material.hpp"
#include "Engine/Rendering/Animation/Skeleton.hpp"
#include "Engine/Rendering/Animation/AnimationClip.hpp"
#include "Engine/Rendering/Core/ForwardRenderingPath.hpp"
#include "Engine/Rendering/Particles/ParticleEmitter.hpp"
#include "Engine/Rendering/DebugRendering/DebugRenderSystem.hpp"

#include "Engine/Core/Time/ScopedProfiler.hpp"

//-----------------------------------------------------------------------------------------------
// Base constructor
//
GameState_Playing::GameState_Playing()
{
}


//-----------------------------------------------------------------------------------------------
// Base destructor
//
GameState_Playing::~GameState_Playing()
{
}


//-----------------------------------------------------------------------------------------------
// Sets up state before updating
//
void GameState_Playing::Enter()
{
 	// Set up the game camera
	Renderer* renderer = Renderer::GetInstance();
	m_gameCamera = new Camera();
	m_gameCamera->SetColorTarget(renderer->GetDefaultColorTarget());
	m_gameCamera->SetDepthTarget(renderer->GetDefaultDepthTarget());
	m_gameCamera->SetProjectionPerspective(45.f, 0.1f, 10000.f);
	m_gameCamera->LookAt(Vector3(0.f, 0.f, -5.0f), Vector3::ZERO);

	Game::GetRenderScene()->AddCamera(m_gameCamera);
	Game::GetRenderScene()->AddLight(Light::CreateDirectionalLight(Vector3::ZERO, Vector3(1.f, -1.f, 0.f), Rgba(255, 255, 255, 0)));
	Game::GetRenderScene()->SetAmbience(Rgba(255, 255, 255, 160));
 
 	// Set up the mouse for FPS controls
	Mouse& mouse = InputSystem::GetMouse();

	mouse.ShowMouseCursor(false);
	mouse.LockCursorToClient(true);
	mouse.SetCursorMode(CURSORMODE_RELATIVE);
 
 	DebugRenderSystem::SetWorldCamera(m_gameCamera);

	AssimpLoader loader;
	loader.OpenFile("Data/Models/unitychan.fbx");
	m_chanSkeleton = loader.ImportSkeleton();
	m_chanModel = loader.ImportMesh(m_chanSkeleton);
	loader.CloseFile();

	loader.OpenFile("Data/Models/unitychan_idle.fbx");
	m_chanIdle = loader.ImportAnimation(m_chanSkeleton, 1)[0];
	loader.CloseFile();

	loader.OpenFile("Data/Models/unitychan_run.fbx");
	m_chanRun = loader.ImportAnimation(m_chanSkeleton, 2)[0];
	loader.CloseFile();

// 	AssimpLoader loader;
// 
// 	{
// 		loader.OpenFile("Data/Models/Maya.fbx");
// 		m_mayaSkeleton = loader.ImportSkeleton();
// 		m_mayaModel = loader.ImportMesh(m_mayaSkeleton);
// 
// 
// 		std::vector<AnimationClip*> animations = loader.ImportAnimation(m_mayaSkeleton, 0);
// 		if (animations.size() > 0)
// 		{
// 			m_mayaAnimation = animations[0];
// 		}
// 		else
// 		{
// 			m_mayaAnimation = nullptr;
// 		}
// 
// 		m_mayaModel->GetSharedMaterial(0)->SetProperty("SPECULAR_POWER", 1.f);
// 		m_mayaModel->GetSharedMaterial(0)->SetProperty("SPECULAR_AMOUNT", 0.0f);
// 		m_mayaModel->AddInstanceMatrix(Matrix44::MakeModelMatrix(Vector3(200.f, 0.f, 0.f), Vector3::ZERO, Vector3::ONES));
// 
// 		loader.CloseFile();
// 	}
// 
// 	
// 
// 	{
// 		loader.OpenFile("Data/Models/Gage.fbx");
// 		m_gageSkeleton = loader.ImportSkeleton();
// 		m_gageModel = loader.ImportMesh(m_gageSkeleton);
// 
// 
// 		std::vector<AnimationClip*> animations = loader.ImportAnimation(m_gageSkeleton, 0);
// 		if (animations.size() > 0)
// 		{
// 			m_gageAnimation = animations[0];
// 		}
// 		else
// 		{
// 			m_gageAnimation = nullptr;
// 		}
// 
// 		m_gageModel->GetSharedMaterial(0)->SetProperty("SPECULAR_POWER", 1.f);
// 		m_gageModel->GetSharedMaterial(0)->SetProperty("SPECULAR_AMOUNT", 0.0f);
// 		m_gageModel->AddInstanceMatrix(Matrix44::MakeModelMatrix(Vector3(0.f, 0.f, 0.f), Vector3::ZERO, Vector3::ONES));
// 
// 		loader.CloseFile();
// 	}
// 
// 	{
// 		loader.OpenFile("Data/Models/Lilith.fbx");
// 		m_lilithSkeleton = loader.ImportSkeleton();
// 		m_lilithModel = loader.ImportMesh(m_lilithSkeleton);
// 
// 
// 		std::vector<AnimationClip*> animations = loader.ImportAnimation(m_lilithSkeleton, 0);
// 		if (animations.size() > 0)
// 		{
// 			m_lilithAnimation = animations[0];
// 		}
// 		else
// 		{
// 			m_lilithAnimation = nullptr;
// 		}
// 
// 		m_lilithModel->GetSharedMaterial(0)->SetProperty("SPECULAR_POWER", 1.f);
// 		m_lilithModel->GetSharedMaterial(0)->SetProperty("SPECULAR_AMOUNT", 0.0f);
// 		m_lilithModel->AddInstanceMatrix(Matrix44::MakeModelMatrix(Vector3(-200.f, 0.f, 0.f), Vector3::ZERO, Vector3::ONES));
// 
// 		loader.CloseFile();
// 	}
// 	
// 	
// 
// 	Game::GetRenderScene()->AddRenderable(m_mayaModel);
// 	Game::GetRenderScene()->AddRenderable(m_gageModel);
// 	Game::GetRenderScene()->AddRenderable(m_lilithModel);

	m_animator.Play(m_chanIdle);
	m_chanModel->AddInstanceMatrix(Matrix44::IDENTITY);
	Game::GetRenderScene()->AddRenderable(m_chanModel);
}


//-----------------------------------------------------------------------------------------------
// Cleans up/finishes any operations before the game leaves this state
//
void GameState_Playing::Leave()
{
	TODO("Clear the render scene");
}


//-----------------------------------------------------------------------------------------------
// Updates the camera transform based on keyboard and mouse input
//
void GameState_Playing::UpdateCameraOnInput()
{
	float deltaTime = Game::GetDeltaTime();
	InputSystem* input = InputSystem::GetInstance();

	// Translating the camera
	Vector3 translationOffset = Vector3::ZERO;
	if (input->IsKeyPressed('W'))								{ translationOffset.z += 1.f; }		// Forward
	if (input->IsKeyPressed('S'))								{ translationOffset.z -= 1.f; }		// Left
	if (input->IsKeyPressed('A'))								{ translationOffset.x -= 1.f; }		// Back
	if (input->IsKeyPressed('D'))								{ translationOffset.x += 1.f; }		// Right
	if (input->IsKeyPressed(InputSystem::KEYBOARD_SPACEBAR))	{ translationOffset.y += 1.f; }		// Up
	if (input->IsKeyPressed('X'))								{ translationOffset.y -= 1.f; }		// Down

	if (input->IsKeyPressed(InputSystem::KEYBOARD_SHIFT))
	{
		translationOffset *= 50.f;
	}

	translationOffset *= CAMERA_TRANSLATION_SPEED * deltaTime;

	m_gameCamera->TranslateLocal(translationOffset);

	// Rotating the camera
	Mouse& mouse = InputSystem::GetMouse();
	IntVector2 mouseDelta = mouse.GetMouseDelta();

	Vector2 rotationOffset = Vector2((float) mouseDelta.y, (float) mouseDelta.x) * 0.12f;
	Vector3 rotation = Vector3(rotationOffset.x * CAMERA_ROTATION_SPEED * deltaTime, rotationOffset.y * CAMERA_ROTATION_SPEED * deltaTime, 0.f);

	m_gameCamera->Rotate(rotation);	
}


//-----------------------------------------------------------------------------------------------
// Checks for input corresponding to toggling rendering
//
void GameState_Playing::CheckRenderInput()
{
	InputSystem* input = InputSystem::GetInstance();

	if (input->WasKeyJustPressed('I'))
	{
		m_renderModel = !m_renderModel;

		if (m_renderModel)
		{
			Game::GetRenderScene()->RemoveRenderable(m_lilithModel);
		}
		else
		{
			Game::GetRenderScene()->AddRenderable(m_lilithModel);
		}
	}

	if (input->WasKeyJustPressed('O'))
	{
		m_renderSkeleton = !m_renderSkeleton;
	}

	if (input->WasKeyJustPressed('P'))
	{
		m_renderAnimation = !m_renderAnimation;
	}

	if (input->WasKeyJustPressed('U'))
	{
		AnimationClip* toPlay = (m_isRunning ? m_chanIdle : m_chanRun);
		m_animator.TransitionToClip(toPlay, 0.5f);
		m_isRunning = !m_isRunning;
	}

	if (input->WasKeyJustPressed(InputSystem::KEYBOARD_UP_ARROW))
	{
		m_timeScale += 0.1f;
	}

	if (input->WasKeyJustPressed(InputSystem::KEYBOARD_DOWN_ARROW))
	{
		m_timeScale -= 0.1f;
	}
}


//-----------------------------------------------------------------------------------------------
// Checks for input
//
void GameState_Playing::ProcessInput()
{
	UpdateCameraOnInput();
	CheckRenderInput();
}


//-----------------------------------------------------------------------------------------------
// Updates the play state
//
void GameState_Playing::Update()
{
	Game::GetGameClock()->SetScale(m_timeScale);
// 	if (m_lilithAnimation != nullptr)
// 	{
// 		int drawCount = m_lilithModel->GetDrawCountPerInstance();
// 		Pose* pose = m_lilithAnimation->CalculatePoseAtTime(Game::GetGameClock()->GetTotalSeconds());
// 
// 		int numBones = pose->GetBoneCount();
// 
// 		for (int i = 0; i < pose->GetBoneCount(); ++i)
// 		{
// 			pose->SetBoneTransform(i, pose->GetBoneTransform(i) * pose->GetBaseSkeleton()->GetBoneData(i).meshToBoneMatrix);
// 		}
// 
// 		for (int i = 0; i < drawCount; ++i)
// 		{
// 			RenderableDraw_t draw = m_lilithModel->GetDraw(i);
// 			draw.sharedMaterial->SetPropertyBlock("boneUBO", pose->GetBoneTransformData(), sizeof(Matrix44) * numBones);
// 		}
// 
// 		delete pose;
// 	}
// 
// 	if (m_mayaAnimation != nullptr)
// 	{
// 		int drawCount = m_mayaModel->GetDrawCountPerInstance();
// 		Pose* pose = m_mayaAnimation->CalculatePoseAtTime(Game::GetGameClock()->GetTotalSeconds());
// 
// 		int numBones = pose->GetBoneCount();
// 
// 		for (int i = 0; i < pose->GetBoneCount(); ++i)
// 		{
// 			pose->SetBoneTransform(i, pose->GetBoneTransform(i) * pose->GetBaseSkeleton()->GetBoneData(i).meshToBoneMatrix);
// 		}
// 
// 		for (int i = 0; i < drawCount; ++i)
// 		{
// 			RenderableDraw_t draw = m_mayaModel->GetDraw(i);
// 			draw.sharedMaterial->SetPropertyBlock("boneUBO", pose->GetBoneTransformData(), sizeof(Matrix44) * numBones);
// 		}
// 
// 		delete pose;
// 	}
// 
// 	if (m_gageAnimation != nullptr)
// 	{
// 		int drawCount = m_gageModel->GetDrawCountPerInstance();
// 		Pose* pose = m_gageAnimation->CalculatePoseAtTime(Game::GetGameClock()->GetTotalSeconds());
// 
// 		int numBones = pose->GetBoneCount();
// 
// 		for (int i = 0; i < pose->GetBoneCount(); ++i)
// 		{
// 			pose->SetBoneTransform(i, pose->GetBoneTransform(i) * pose->GetBaseSkeleton()->GetBoneData(i).meshToBoneMatrix);
// 		}
// 
// 		for (int i = 0; i < drawCount; ++i)
// 		{
// 			RenderableDraw_t draw = m_gageModel->GetDraw(i);
// 			draw.sharedMaterial->SetPropertyBlock("boneUBO", pose->GetBoneTransformData(), sizeof(Matrix44) * numBones);
// 		}
// 
// 		delete pose;
// 	}

	int drawCount = m_chanModel->GetDrawCountPerInstance();
	Pose* pose = m_animator.GetCurrentPose();
	
	int numBones = pose->GetBoneCount();
	
	for (int i = 0; i < pose->GetBoneCount(); ++i)
	{
		pose->SetBoneTransform(i, pose->GetBoneTransform(i) * pose->GetBaseSkeleton()->GetBoneData(i).meshToBoneMatrix);
	}
	
	for (int i = 0; i < drawCount; ++i)
	{
		RenderableDraw_t draw = m_chanModel->GetDraw(i);
		draw.sharedMaterial->SetPropertyBlock("boneUBO", pose->GetBoneTransformData(), sizeof(Matrix44) * numBones);
	}
}


//-----------------------------------------------------------------------------------------------
// Renders the gameplay state to screen
//
void GameState_Playing::Render() const
{
	ForwardRenderingPath::Render(Game::GetRenderScene());

// 	if (m_renderSkeleton)
// 	{
//  		DebugRenderSystem::DrawSkeleton(m_lilithSkeleton, Matrix44::IDENTITY, 0.f);
//  	}

// 	if (m_renderAnimation && m_lilithAnimation != nullptr)
// 	{
// 		//float time = Game::GetGameClock()->GetTotalSeconds();
// 		float time = 0.f;
// 		Pose* pose = m_lilithAnimation->CalculatePoseAtTime(time);
// 
// 		int numBones = pose->GetBoneCount();
// 
// 		for (int boneIndex = 0; boneIndex < numBones; ++boneIndex)
// 		{
// 			const SkeletonBase* base = pose->GetBaseSkeleton();
// 
// 			BoneData_t currentBone = base->GetBoneData(boneIndex);
// 			Matrix44 poseTransform = pose->GetBoneTransform(boneIndex);
// 
// 			Vector3 currPos = Matrix44::ExtractTranslation(poseTransform);
// 
// 			int parentIndex = currentBone.parentIndex;
// 
// 			// Root
// 			if (parentIndex >= 0)
// 			{
// 				Matrix44 parentTransform = pose->GetBoneTransform(parentIndex);
// 				Vector3 parentPos = Matrix44::ExtractTranslation(parentTransform);
// 
// 				DebugRenderSystem::DrawCube(currPos, 0.f, Rgba::WHITE);
// 				DebugRenderSystem::Draw3DLine(currPos, parentPos, Rgba::WHITE, 0.f);
// 			}
// 		}
// 
// 		delete pose;
// 	}
}
