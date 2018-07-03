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
#include "Engine/Rendering/Animation/SkeletonBase.hpp"
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
	m_gameCamera->SetProjectionPerspective(45.f, 0.1f, 1000.f);
	m_gameCamera->LookAt(Vector3(0.f, 0.f, -5.0f), Vector3::ZERO);

	Game::GetRenderScene()->AddCamera(m_gameCamera);
	Game::GetRenderScene()->AddLight(Light::CreateDirectionalLight(Vector3::ZERO, Vector3::DIRECTION_DOWN, Rgba(255, 255, 255, 100)));
	Game::GetRenderScene()->SetAmbience(Rgba(255, 255, 255, 100));
 
 	// Set up the mouse for FPS controls
	Mouse& mouse = InputSystem::GetMouse();

	mouse.ShowMouseCursor(false);
	mouse.LockCursorToClient(true);
	mouse.SetCursorMode(CURSORMODE_RELATIVE);
 
 	DebugRenderSystem::SetWorldCamera(m_gameCamera);

	AssimpLoader loader;
	//loader.LoadFile("Data/Models/Gage/Gage.fbx");

	//loader.LoadFile("Data/Models/unitychan.fbx");
	//loader.LoadFile("Data/Models/lilith.fbx");
	//loader.LoadFile("Data/Models/maya.fbx");


	loader.LoadFile("Data/Models/lilith2.fbx");
	m_clip = loader.GetAnimationClip(0);

	m_modelRenderable = loader.GetRenderable();
	m_skeleton = loader.GetSkeletonBase();

	//Renderable* hello = AssetDB::LoadModelWithAssimp("Data/Models/unitychan_ARpose1.fbx", false);

	//Renderable* katsuragi = AssetDB::LoadModelWithAssimp("Data/Models/Katsuragi/Katsuragi.obj", false);

	//katsuragi->GetSharedMaterial(0)->SetProperty("SPECULAR_POWER", 20.f);
	//katsuragi->GetSharedMaterial(0)->SetProperty("SPECULAR_AMOUNT", 0.2f);
	//katsuragi->AddInstanceMatrix(Matrix44::MakeModelMatrix(Vector3(-100.f, 0.f, 0.f), Vector3::ZERO, Vector3(500.f, 500.f, 500.f)));

	m_modelRenderable->GetSharedMaterial(0)->SetProperty("SPECULAR_POWER", 20.f);
	m_modelRenderable->GetSharedMaterial(0)->SetProperty("SPECULAR_AMOUNT", 0.2f);
	m_modelRenderable->AddInstanceMatrix(Matrix44::MakeModelMatrix(Vector3(0.f, 0.f, 0.f), Vector3::ZERO, Vector3::ONES));

	//Game::GetRenderScene()->AddRenderable(katsuragi);
	Game::GetRenderScene()->AddRenderable(m_modelRenderable);

	RenderableDraw_t draw;
	draw.mesh = AssetDB::GetMesh("Cube");
	draw.sharedMaterial = AssetDB::GetSharedMaterial("Default_Opaque");
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
			Game::GetRenderScene()->RemoveRenderable(m_modelRenderable);
		}
		else
		{
			Game::GetRenderScene()->AddRenderable(m_modelRenderable);
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
	int drawCount = m_modelRenderable->GetDrawCountPerInstance();
	Pose* pose = m_clip->CalculatePoseAtTime(Game::GetGameClock()->GetTotalSeconds());
	int numBones = pose->GetBoneCount();

	for (int i = 0; i < pose->GetBoneCount(); ++i)
	{
		pose->SetBoneTransform(i, pose->GetBoneTransform(i) * pose->GetBaseSkeleton()->GetBoneData(i).meshToBoneMatrix);
		//pose->SetBoneTransform(i, Matrix44::IDENTITY);
	}

	for (int i = 0; i < drawCount; ++i)
	{
		RenderableDraw_t draw = m_modelRenderable->GetDraw(i);
		draw.sharedMaterial->SetPropertyBlock("boneUBO", pose->GetBoneTransformData(), sizeof(Matrix44) * numBones);
	}

	delete pose;

}


//-----------------------------------------------------------------------------------------------
// Renders the gameplay state to screen
//
void GameState_Playing::Render() const
{
	ForwardRenderingPath::Render(Game::GetRenderScene());

	if (m_renderSkeleton)
	{
 		DebugRenderSystem::DrawSkeleton(m_skeleton, Matrix44::IDENTITY, 0.f);
 	}

	if (m_renderAnimation)
	{
		float time = Game::GetGameClock()->GetTotalSeconds();

		Pose* pose = m_clip->CalculatePoseAtTime(time);

		int numBones = pose->GetBoneCount();

		for (int boneIndex = 0; boneIndex < numBones; ++boneIndex)
		{
			const SkeletonBase* base = pose->GetBaseSkeleton();

			BoneData_t currentBone = base->GetBoneData(boneIndex);
			Matrix44 poseTransform = pose->GetBoneTransform(boneIndex);

			Vector3 currPos = Matrix44::ExtractTranslation(poseTransform);

			int parentIndex = currentBone.parentIndex;

			// Root
			if (parentIndex >= 0)
			{
				Matrix44 parentTransform = pose->GetBoneTransform(parentIndex);
				Vector3 parentPos = Matrix44::ExtractTranslation(parentTransform);

				DebugRenderSystem::DrawCube(currPos, 0.f, Rgba::WHITE);
				DebugRenderSystem::Draw3DLine(currPos, parentPos, Rgba::WHITE, 0.f);
			}
		}

		delete pose;
	}
}
