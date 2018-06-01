/************************************************************************/
/* File: GameState_Playing.cpp
/* Author: Andrew Chase
/* Date: May 21st, 2018
/* Description: Implementation of the GameState_Playing class
/************************************************************************/
#include "Game/App.hpp"
#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"
#include "Game/GameState_Playing.hpp"

#include "Engine/Core/AssetDB.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/Sampler.hpp"
#include "Engine/Renderer/Material.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/MeshGroup.hpp"
#include "Engine/Renderer/Renderable.hpp"
#include "Engine/Renderer/RenderScene.hpp"
#include "Engine/Renderer/ParticleEmitter.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "Engine/Renderer/ForwardRenderingPath.hpp"

// Constants
const float GameState_Playing::CAMERA_ROTATION_SPEED = 45.f;
const float GameState_Playing::CAMERA_TRANSLATION_SPEED = 10.f;

//-----------------------------------------------------------------------------------------------
// Base constructor
//
GameState_Playing::GameState_Playing()
{
	m_scene = new RenderScene("Default");
}


//-----------------------------------------------------------------------------------------------
// Base destructor
//
GameState_Playing::~GameState_Playing()
{
	delete m_emitter;
	m_emitter = nullptr;

	delete m_scene;
	m_scene = nullptr;
}


//-----------------------------------------------------------------------------------------------
// Sets up state before updating
//
void GameState_Playing::Enter()
{
	MeshGroup* group = AssetDB::CreateOrGetMeshGroup("Miku.obj");

	Renderable* renderable = new Renderable(Matrix44::IDENTITY, group, nullptr);

	Material* quad		= AssetDB::CreateOrGetSharedMaterial("Miku_Quad_Instanced"); 
	Material* base		= AssetDB::CreateOrGetSharedMaterial("Miku_Base_Instanced"); 
	Material* detail	= AssetDB::CreateOrGetSharedMaterial("Miku_Detail_Instanced");

	// Make a linear sampler for miku
	Sampler* sampler = new Sampler();
	sampler->Initialize(SAMPLER_FILTER_LINEAR, EDGE_SAMPLING_REPEAT);

	quad->SetSampler(0, sampler);
	base->SetSampler(0, sampler);
	detail->SetSampler(0, sampler);

	quad->SetProperty("SPECULAR_POWER", 25.0f);
	quad->SetProperty("SPECULAR_AMOUNT", 1.f);
	base->SetProperty("SPECULAR_POWER", 25.0f);
	base->SetProperty("SPECULAR_AMOUNT", 1.f);	
	detail->SetProperty("SPECULAR_POWER", 25.0f);
	detail->SetProperty("SPECULAR_AMOUNT", 1.f);

	renderable->SetSharedMaterial(quad, 0);
	renderable->SetSharedMaterial(base, 1);
	renderable->SetSharedMaterial(detail, 2);
	renderable->SetSharedMaterial(base, 3);

	m_emitter = new ParticleEmitter(Game::GetGameClock());
	m_emitter->SetRenderable(renderable);
	renderable->ClearInstances();

	m_scene->AddRenderable(renderable);

	m_emitter->SetSpawnRate(100);
	m_emitter->SetSpawnVelocityFunction([]() { return 10.f * Vector3(GetRandomFloatInRange(-6.f, 6.f), 6.f, GetRandomFloatInRange(-6.f, 6.f)); });
	m_emitter->SetSpawnAngularVelocityFunction([]() { return 360.f * GetRandomPointOnSphere(); });
	m_emitter->SetSpawnLifetimeFunction([]() { return 5.f; });

	// Set up the game camera
	Renderer* renderer = Renderer::GetInstance();
	m_gameCamera = new Camera();
	m_gameCamera->SetColorTarget(renderer->GetDefaultColorTarget());
	m_gameCamera->SetDepthTarget(renderer->GetDefaultDepthTarget());
	m_gameCamera->SetProjectionPerspective(45.f, 0.1f, 1000.f);
	m_gameCamera->LookAt(Vector3(0.f, 0.f, -5.0f), Vector3::ZERO);

	m_scene->AddCamera(m_gameCamera);
	m_scene->AddLight(Light::CreateDirectionalLight(Vector3::ZERO));
	m_scene->SetAmbience(Rgba(100, 100, 100, 255));

	// Set up the mouse for FPS controls
	Mouse& mouse = InputSystem::GetMouse();

	mouse.ShowMouseCursor(false);
	mouse.LockCursorToClient(true);
	mouse.SetCursorMode(CURSORMODE_RELATIVE);

	DebugRenderSystem::SetWorldCamera(m_gameCamera);

	DebugRenderSystem::DrawUVSphere(Vector3::ZERO, 300.f);
}


//-----------------------------------------------------------------------------------------------
// Cleans up/finishes any operations before the game leaves this state
//
void GameState_Playing::Leave()
{
}


//-----------------------------------------------------------------------------------------------
// Updates the camera based on mouse and keyboard input
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
// Checks for input
//
void GameState_Playing::ProcessInput()
{
	UpdateCameraOnInput();
}


//-----------------------------------------------------------------------------------------------
// Updates the play state
//
void GameState_Playing::Update()
{
	m_emitter->Update();
}


//-----------------------------------------------------------------------------------------------
// Renders the gameplay state to screen
//
void GameState_Playing::Render() const
{
	ForwardRenderingPath::Render(m_scene);
}
