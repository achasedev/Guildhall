/************************************************************************/
/* File: Player.cpp
/* Author: Andrew Chase
/* Date: June 5th, 2018
/* Description: Implementation of the Player class
/************************************************************************/
#include "Game/Entity/Player.hpp"
#include "Game/Framework/Game.hpp"
#include "Game/Environment/Map.hpp"
#include "Engine/Assets/AssetDB.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/Time/Stopwatch.hpp"
#include "Engine/Rendering/Core/Renderer.hpp"
#include "Engine/Rendering/Core/OrbitCamera.hpp"
#include "Engine/Rendering/DebugRendering/DebugRenderSystem.hpp"

// Constants
const float Player::PLAYER_ROTATION_SPEED = 45.f;
const float Player::PLAYER_TRANSLATION_SPEED = 10.f;


//-----------------------------------------------------------------------------------------------
// Constructor
//
Player::Player()
{
	Renderer* renderer = Renderer::GetInstance();

	// Set up the player camera
	m_camera = new OrbitCamera();
	m_camera->SetColorTarget(renderer->GetDefaultColorTarget());
	m_camera->SetDepthTarget(renderer->GetDefaultDepthTarget());
	m_camera->SetTarget(transform.position);
	m_camera->SetRadiusLimits(5.f, 25.f);
	m_camera->SetRadius(10.f);
	m_camera->SetAzimuthLimits(5.f, 135.f);
	m_camera->SetAzimuth(45.f);
	m_camera->SetProjectionPerspective(90.f, 0.1f, 1000.f);

	// Set up the renderable
	MeshGroup* mikuMesh = AssetDB::CreateOrGetMeshGroup("Miku.obj");
	Material* baseMaterial = AssetDB::CreateOrGetSharedMaterial("Miku_Base");
	Material* quadMaterial = AssetDB::CreateOrGetSharedMaterial("Miku_Quad");
	Material* detailMaterial = AssetDB::CreateOrGetSharedMaterial("Miku_Detail");

	m_renderable = Renderable(transform.GetModelMatrix(), mikuMesh, baseMaterial);
	m_renderable.SetSharedMaterial(quadMaterial, 0);
	m_renderable.SetSharedMaterial(detailMaterial, 2);

	transform.position = Vector3::ZERO;
	m_stopwatch = new Stopwatch(Game::GetGameClock());
	m_stopwatch->SetInterval(0.5f);
}


//-----------------------------------------------------------------------------------------------
// Destructor
//
Player::~Player()
{
	delete m_stopwatch;
	m_stopwatch = nullptr;

	delete m_camera;
	m_camera = nullptr;
}


//-----------------------------------------------------------------------------------------------
// Checks for input and updates the player and camera transforms
//
void Player::ProcessInput()
{
	float deltaTime = m_stopwatch->GetDeltaSeconds();

	UpdatePositionOnInput(deltaTime);
	UpdateCameraOnInput(deltaTime);

	Mouse& mouse = InputSystem::GetMouse();
	if (mouse.IsButtonPressed(MOUSEBUTTON_RIGHT))
	{
		transform.rotation.y = m_camera->GetRotation().y;
	}

	m_camera->SetTarget(transform.position);
}


//-----------------------------------------------------------------------------------------------
// Sets the actor's position to match the height of th map and spawns the breadcrumbs
//
void Player::Update(float deltaTime)
{
	GameObject::Update(deltaTime);

	UpdateHeightOnMap();

	m_renderable.SetModelMatrix(transform.GetModelMatrix(), 0);

	// Drop a breadcrumb if the timer is up
	if (m_stopwatch->DecrementByIntervalOnce())
	{
		DebugRenderOptions options;
		options.m_startColor = Rgba::DARK_GREEN;
		options.m_endColor = Rgba::RED;
		options.m_lifetime = 4.f;
		
		DebugRenderSystem::DrawPoint(transform.position, options, 0.2f);
	}
}


//-----------------------------------------------------------------------------------------------
// Returns the player's camera
//
Camera* Player::GetCamera() const
{
	return m_camera;
}


//-----------------------------------------------------------------------------------------------
// Updates the camera's transform based on mouse input
//
void Player::UpdateCameraOnInput(float deltaTime)
{
	// Rotating the camera
	Mouse& mouse = InputSystem::GetMouse();
	IntVector2 mouseDelta = mouse.GetMouseDelta();

	Vector2 rotationOffset = Vector2((float) mouseDelta.y, (float) mouseDelta.x) * 0.12f;
	Vector3 rotation = Vector3(rotationOffset.x * PLAYER_ROTATION_SPEED * deltaTime, rotationOffset.y * PLAYER_ROTATION_SPEED * deltaTime, 0.f);

	m_camera->RotateHorizontally(-rotation.y);
	m_camera->RotateVertically(-rotation.x);

	float wheelDelta = mouse.GetMouseWheelDelta();
	if (wheelDelta != 0.f)
	{
		m_camera->MoveAlongRadius(wheelDelta * 10.f);
	}
}


//-----------------------------------------------------------------------------------------------
// Updates the player's transform based on keyboard input
//
void Player::UpdatePositionOnInput(float deltaTime)
{
	InputSystem* input = InputSystem::GetInstance();

	// Translating the camera
	Vector3 translationOffset = Vector3::ZERO;
	if (input->IsKeyPressed('W'))								{ translationOffset.z += 1.f; }		// Forward
	if (input->IsKeyPressed('S'))								{ translationOffset.z -= 1.f; }		// Left
	if (input->IsKeyPressed('A'))								{ translationOffset.x -= 1.f; }		// Back
	if (input->IsKeyPressed('D'))								{ translationOffset.x += 1.f; }		// Right

	translationOffset *= PLAYER_TRANSLATION_SPEED * deltaTime;

	transform.TranslateLocal(translationOffset);
}


//-----------------------------------------------------------------------------------------------
// Finds the height of the terrain at the player's postion and set the player to be at that height
//
void Player::UpdateHeightOnMap()
{
	Map* map = Game::GetMap();
	float height = map->GetHeightAtPosition(transform.position);
	transform.position.y = height;
}
