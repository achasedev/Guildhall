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
	m_gameCamera->LookAt(Vector3(0.f, 200.f, -500.0f), Vector3(0.f, 200.f, 0.f));

	Game::GetRenderScene()->AddCamera(m_gameCamera);
	Game::GetRenderScene()->AddLight(Light::CreateDirectionalLight(Vector3::ZERO, Vector3(1.f, -1.f, 0.f), Rgba(255, 255, 255, 0)));
	Game::GetRenderScene()->SetAmbience(Rgba(255, 255, 255, 160));
 
 	// Set up the mouse for FPS controls
	Mouse& mouse = InputSystem::GetMouse();

	mouse.ShowMouseCursor(false);
	mouse.LockCursorToClient(true);
	mouse.SetCursorMode(CURSORMODE_RELATIVE);
 
 	DebugRenderSystem::SetWorldCamera(m_gameCamera);

	LoadGage();
	LoadMaya();
	LoadUnityChan();

 	Game::GetRenderScene()->AddRenderable(m_gageModel);
 	Game::GetRenderScene()->AddRenderable(m_mayaModel);
	Game::GetRenderScene()->AddRenderable(m_chanModel);

	m_chanAnimator.Play(m_chanIdle);
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
		m_renderModels = !m_renderModels;

		if (m_renderModels)
		{
			Game::GetRenderScene()->AddRenderable(m_gageModel);
			Game::GetRenderScene()->AddRenderable(m_mayaModel);
			Game::GetRenderScene()->AddRenderable(m_chanModel);
		}
		else
		{
			Game::GetRenderScene()->RemoveRenderable(m_gageModel);
			Game::GetRenderScene()->RemoveRenderable(m_mayaModel);
			Game::GetRenderScene()->RemoveRenderable(m_chanModel);
		}
	}

	if (input->WasKeyJustPressed('O'))
	{
		m_renderSkeletons = !m_renderSkeletons;
	}

	if (input->WasKeyJustPressed('P'))
	{
		m_renderAnimations = !m_renderAnimations;
	}

	if (input->WasKeyJustPressed('U'))
	{
		AnimationClip* toPlay = (m_isRunning ? m_chanIdle : m_chanRun);
		m_chanAnimator.TransitionToClip(toPlay, 0.5f);
		m_isRunning = !m_isRunning;
	}
}


void GameState_Playing::LoadGage()
{
	AssimpLoader loader;
	loader.OpenFile("Data/Models/Gage.fbx");
	m_gageSkeleton = loader.ImportSkeleton();
	m_gageModel = loader.ImportMesh(m_gageSkeleton);
	 
	std::vector<AnimationClip*> animations = loader.ImportAnimation(m_gageSkeleton, 0);
	m_gageAnimation = animations[0];
	
	Matrix44 model = Matrix44::MakeModelMatrix(Vector3(-300.f, 0.f, 0.f), Vector3(0.f, -45.f, 0.f), Vector3::ONES);
	m_gageModel->AddInstanceMatrix(model);

	loader.CloseFile();
}

void GameState_Playing::LoadMaya()
{
	AssimpLoader loader;
	loader.OpenFile("Data/Models/Maya.fbx");
	m_mayaSkeleton = loader.ImportSkeleton();
	m_mayaModel = loader.ImportMesh(m_mayaSkeleton);
	
	
	std::vector<AnimationClip*> animations = loader.ImportAnimation(m_mayaSkeleton, 0);
	m_mayaAnimation = animations[0];
	
	Matrix44 model = Matrix44::MakeModelMatrix(Vector3(300.f, 0.f, 0.f), Vector3(0.f, 45.f, 0.f), Vector3::ONES);
	m_mayaModel->AddInstanceMatrix(model);

	loader.CloseFile();
}


void GameState_Playing::LoadUnityChan()
{
	AssimpLoader loader;
	loader.OpenFile("Data/Models/unitychan.fbx");
	m_chanSkeleton = loader.ImportSkeleton();
	m_chanModel = loader.ImportMesh(m_chanSkeleton);
	loader.CloseFile();

	loader.OpenFile("Data/Models/unitychan_idle.fbx");
	m_chanIdle = loader.ImportAnimation(m_chanSkeleton, 1)[0];
	loader.CloseFile();

	loader.OpenFile("Data/Models/unitychan_run.fbx");
	m_chanRun = loader.ImportAnimation(m_chanSkeleton, 1)[0];

	m_chanModel->AddInstanceMatrix(Matrix44::MakeModelMatrix(Vector3(0.f, 0.f, 300.f), Vector3(0.f, 0.f, 0.f), Vector3(3.f, 3.f, 3.f)));

	loader.CloseFile();

	// Add textures, hacky but adds some color
	// The face won't show up, as it's a morph target
	Texture* bodyTexture = AssetDB::CreateOrGetTexture("Data/Images/UnityChan/Body.tga");
	Texture* skinTexture = AssetDB::CreateOrGetTexture("Data/Images/UnityChan/Skin.tga");

	RenderableDraw_t draw = m_chanModel->GetDraw(16);
	draw.sharedMaterial->SetDiffuse(bodyTexture);
	m_chanModel->SetSharedMaterial(16, draw.sharedMaterial);

	draw = m_chanModel->GetDraw(8);
	draw.sharedMaterial->SetDiffuse(skinTexture);
	m_chanModel->SetSharedMaterial(8, draw.sharedMaterial);

	draw = m_chanModel->GetDraw(9);
	draw.sharedMaterial->SetDiffuse(skinTexture);
	m_chanModel->SetSharedMaterial(9, draw.sharedMaterial);

	draw = m_chanModel->GetDraw(10);
	draw.sharedMaterial->SetDiffuse(skinTexture);
	m_chanModel->SetSharedMaterial(10, draw.sharedMaterial);

	draw = m_chanModel->GetDraw(11);
	draw.sharedMaterial->SetDiffuse(skinTexture);
	m_chanModel->SetSharedMaterial(11, draw.sharedMaterial);

	draw = m_chanModel->GetDraw(13);
	draw.sharedMaterial->SetDiffuse(skinTexture);
	m_chanModel->SetSharedMaterial(13, draw.sharedMaterial);


	draw = m_chanModel->GetDraw(14);
	draw.sharedMaterial->SetDiffuse(bodyTexture);
	m_chanModel->SetSharedMaterial(14, draw.sharedMaterial);

	draw = m_chanModel->GetDraw(15);
	draw.sharedMaterial->SetDiffuse(bodyTexture);
	m_chanModel->SetSharedMaterial(15, draw.sharedMaterial);

	draw = m_chanModel->GetDraw(16);
	draw.sharedMaterial->SetDiffuse(bodyTexture);
	m_chanModel->SetSharedMaterial(16, draw.sharedMaterial);

	draw = m_chanModel->GetDraw(17);
	draw.sharedMaterial->SetDiffuse(bodyTexture);
	m_chanModel->SetSharedMaterial(17, draw.sharedMaterial);

	draw = m_chanModel->GetDraw(18);
	draw.sharedMaterial->SetDiffuse(bodyTexture);
	m_chanModel->SetSharedMaterial(18, draw.sharedMaterial);

	draw = m_chanModel->GetDraw(20);
	draw.sharedMaterial->SetDiffuse(bodyTexture);
	m_chanModel->SetSharedMaterial(20, draw.sharedMaterial);

	draw = m_chanModel->GetDraw(22);
	draw.sharedMaterial->SetDiffuse(bodyTexture);
	m_chanModel->SetSharedMaterial(22, draw.sharedMaterial);
}

void GameState_Playing::UpdateGage()
{
	int drawCount = m_gageModel->GetDrawCountPerInstance();
	Pose* pose = m_gageAnimation->CalculatePoseAtTime(Game::GetGameClock()->GetTotalSeconds());
	
	int numBones = pose->GetBoneCount();
	
	for (int i = 0; i < pose->GetBoneCount(); ++i)
	{
		pose->SetBoneTransform(i, pose->GetBoneTransform(i) * pose->GetSkeleton()->GetBoneData(i).meshToBoneMatrix);
	}
	
	for (int i = 0; i < drawCount; ++i)
	{
		RenderableDraw_t draw = m_gageModel->GetDraw(i);
		draw.sharedMaterial->SetPropertyBlock("boneUBO", pose->GetTotalBoneData(), sizeof(Matrix44) * numBones);
	}
	
	delete pose;
}

void GameState_Playing::UpdateMaya()
{
	int drawCount = m_mayaModel->GetDrawCountPerInstance();
	Pose* pose = m_mayaAnimation->CalculatePoseAtTime(Game::GetGameClock()->GetTotalSeconds());
	
	int numBones = pose->GetBoneCount();
	
	for (int i = 0; i < pose->GetBoneCount(); ++i)
	{
		pose->SetBoneTransform(i, pose->GetBoneTransform(i) * pose->GetSkeleton()->GetBoneData(i).meshToBoneMatrix);
	}
	
	for (int i = 0; i < drawCount; ++i)
	{
		RenderableDraw_t draw = m_mayaModel->GetDraw(i);
		draw.sharedMaterial->SetPropertyBlock("boneUBO", pose->GetTotalBoneData(), sizeof(Matrix44) * numBones);
	}
	
	delete pose;
}


void GameState_Playing::UpdateUnityChan()
{
	int drawCount = m_chanModel->GetDrawCountPerInstance();
	Pose* pose = m_chanAnimator.GetCurrentPose();

	int numBones = pose->GetBoneCount();

	for (int i = 0; i < pose->GetBoneCount(); ++i)
	{
		pose->SetBoneTransform(i, pose->GetBoneTransform(i) * pose->GetSkeleton()->GetBoneData(i).meshToBoneMatrix);
	}

	for (int i = 0; i < drawCount; ++i)
	{
		RenderableDraw_t draw = m_chanModel->GetDraw(i);
		draw.sharedMaterial->SetPropertyBlock("boneUBO", pose->GetTotalBoneData(), sizeof(Matrix44) * numBones);
	}
}

void GameState_Playing::RenderGageAnim() const
{
	float time = Game::GetGameClock()->GetTotalSeconds();

	Pose* gagePose = m_gageAnimation->CalculatePoseAtTime(time);

	int numBones = gagePose->GetBoneCount();

	MeshBuilder mb;
	mb.BeginBuilding(PRIMITIVE_LINES, false);
	for (int boneIndex = 0; boneIndex < numBones; ++boneIndex)
	{
		const Skeleton* skeleton = gagePose->GetSkeleton();

		BoneData_t currentBone = skeleton->GetBoneData(boneIndex);
		Matrix44 poseTransform = m_gageModel->GetInstanceMatrix(0) * gagePose->GetBoneTransform(boneIndex);

		Vector3 currPos = Matrix44::ExtractTranslation(poseTransform);

		int parentIndex = currentBone.parentIndex;

		// Root
		if (parentIndex >= 0)
		{
			Matrix44 parentTransform = m_gageModel->GetInstanceMatrix(0) * gagePose->GetBoneTransform(parentIndex);
			Vector3 parentPos = Matrix44::ExtractTranslation(parentTransform);

			mb.PushLine(currPos, parentPos);
			//DebugRenderSystem::DrawCube(currPos, 0.f, Rgba::WHITE);
			//DebugRenderSystem::Draw3DLine(currPos, parentPos, Rgba::WHITE, 0.f);
		}
	}

	mb.FinishBuilding();
	Mesh* mesh = mb.CreateMesh<Vertex3D_PCU>();
	Renderer* renderer = Renderer::GetInstance();
	renderer->DrawMesh(mesh);
	delete mesh;

	delete gagePose;
}

void GameState_Playing::RenderMayaAnim() const
{
	float time = Game::GetGameClock()->GetTotalSeconds();

	Pose* mayaPose = m_mayaAnimation->CalculatePoseAtTime(time);

	int numBones = mayaPose->GetBoneCount();

	MeshBuilder mb;
	mb.BeginBuilding(PRIMITIVE_LINES, false);
	for (int boneIndex = 0; boneIndex < numBones; ++boneIndex)
	{
		const Skeleton* skeleton = mayaPose->GetSkeleton();

		BoneData_t currentBone = skeleton->GetBoneData(boneIndex);
		Matrix44 poseTransform = m_mayaModel->GetInstanceMatrix(0) * mayaPose->GetBoneTransform(boneIndex);

		Vector3 currPos = Matrix44::ExtractTranslation(poseTransform);

		int parentIndex = currentBone.parentIndex;

		// Root
		if (parentIndex >= 0)
		{
			Matrix44 parentTransform = m_mayaModel->GetInstanceMatrix(0) * mayaPose->GetBoneTransform(parentIndex);
			Vector3 parentPos = Matrix44::ExtractTranslation(parentTransform);

			{
				mb.PushLine(parentPos, currPos);
				//DebugRenderSystem::DrawCube(currPos, 0.f, Rgba::WHITE);
				//DebugRenderSystem::Draw3DLine(currPos, parentPos, Rgba::WHITE, 0.f);
			}
		}
	}

	mb.FinishBuilding();
	Mesh* mesh = mb.CreateMesh<Vertex3D_PCU>();
	Renderer* renderer = Renderer::GetInstance();
	renderer->DrawMesh(mesh);
	delete mesh;

	delete mayaPose;

}


void GameState_Playing::RenderUnityChanAnim() const
{
	Pose* chanAnim = m_chanAnimator.GetCurrentPose();

	int numBones = chanAnim->GetBoneCount();

	MeshBuilder mb;
	mb.BeginBuilding(PRIMITIVE_LINES, false);
	for (int boneIndex = 0; boneIndex < numBones; ++boneIndex)
	{
		const Skeleton* skeleton = chanAnim->GetSkeleton();

		BoneData_t currentBone = skeleton->GetBoneData(boneIndex);
		Matrix44 poseTransform = m_chanModel->GetInstanceMatrix(0) * chanAnim->GetBoneTransform(boneIndex);

		Vector3 currPos = Matrix44::ExtractTranslation(poseTransform);

		int parentIndex = currentBone.parentIndex;

		// Root
		if (parentIndex >= 0)
		{
			Matrix44 parentTransform = m_chanModel->GetInstanceMatrix(0) * chanAnim->GetBoneTransform(parentIndex);
			Vector3 parentPos = Matrix44::ExtractTranslation(parentTransform);

			mb.PushLine(currPos, parentPos);
			//DebugRenderSystem::DrawCube(currPos, 0.f, Rgba::WHITE);
			//DebugRenderSystem::Draw3DLine(currPos, parentPos, Rgba::WHITE, 0.f);
		}
	}

	mb.FinishBuilding();
	Mesh* mesh = mb.CreateMesh<Vertex3D_PCU>();
	Renderer* renderer = Renderer::GetInstance();
	renderer->DrawMesh(mesh);
	delete mesh;

	delete chanAnim;
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
	UpdateGage();
	UpdateMaya();
	UpdateUnityChan();
}


//-----------------------------------------------------------------------------------------------
// Renders the gameplay state to screen
//
void GameState_Playing::Render() const
{
	ForwardRenderingPath::Render(Game::GetRenderScene());
	

 	if (m_renderSkeletons)
 	{
		Renderer* renderer = Renderer::GetInstance();

		DebugRenderSystem::DrawSkeleton(m_gageSkeleton,		m_gageModel->GetInstanceMatrix(0), 0.f);
		DebugRenderSystem::DrawSkeleton(m_mayaSkeleton,		m_mayaModel->GetInstanceMatrix(0), 0.f);
		DebugRenderSystem::DrawSkeleton(m_chanSkeleton,		m_chanModel->GetInstanceMatrix(0), 0.f);
	}

	if (m_renderAnimations)
	{
		RenderGageAnim();
		RenderMayaAnim();
		RenderUnityChanAnim();
	}
}
