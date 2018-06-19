/************************************************************************/
/* File: Player.cpp
/* Author: Andrew Chase
/* Date: June 5th, 2018
/* Description: Implementation of the Player class
/************************************************************************/
#include "Game/Entity/Player.hpp"
#include "Game/Entity/Turret.hpp"
#include "Game/Entity/Cannon.hpp"
#include "Game/Framework/Game.hpp"
#include "Game/Environment/Map.hpp"

#include "Engine/Core/Window.hpp"
#include "Engine/Assets/AssetDB.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/Time/Stopwatch.hpp"
#include "Engine/Rendering/Core/Renderer.hpp"
#include "Engine/Rendering/Core/Renderable.hpp"
#include "Engine/Rendering/Core/OrbitCamera.hpp"
#include "Engine/Rendering/Core/RenderScene.hpp"
#include "Engine/Rendering/Meshes/MeshGroup.hpp"

#include "Engine/Rendering/DebugRendering/DebugRenderSystem.hpp"

// Constants
const Vector3	Player::CAMERA_TARGET_OFFSET = Vector3(0.f, 5.f, 0.f);
const float		Player::CAMERA_ROTATION_SPEED = 45.f;


//-----------------------------------------------------------------------------------------------
// Constructor
//
Player::Player()
	: Tank(0)
{
	Renderer* renderer = Renderer::GetInstance();

	// Set up the player camera
	m_camera = new OrbitCamera();
	m_camera->SetColorTarget(renderer->GetDefaultColorTarget());
	m_camera->SetDepthTarget(renderer->GetDefaultDepthTarget());
	m_camera->SetTarget(transform.position + CAMERA_TARGET_OFFSET);
	m_camera->SetRadiusLimits(5.f, 25.f);
	m_camera->SetRadius(15.f);
	m_camera->SetAzimuthLimits(5.f, 135.f);
	m_camera->SetAzimuth(45.f);
	m_camera->SetProjectionPerspective(90.f, 0.1f, 1000.f);

	transform.position = Vector3::ZERO;
	m_stopwatch = new Stopwatch(Game::GetGameClock());
	m_stopwatch->SetInterval(0.5f);
}


//-----------------------------------------------------------------------------------------------
// Destructor
//
Player::~Player()
{
	Game::GetRenderScene()->RemoveRenderable(m_renderable);

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

	m_camera->SetTarget(transform.position + CAMERA_TARGET_OFFSET);

	Mouse& mouse = InputSystem::GetMouse();
	if (mouse.WasButtonJustPressed(MOUSEBUTTON_LEFT))
	{
		ShootCannon();
	}
}


//-----------------------------------------------------------------------------------------------
// Sets the actor's position to match the height of th map and spawns the breadcrumbs
//
void Player::Update(float deltaTime)
{
	// Drop a breadcrumb if the timer is up
	if (m_stopwatch->DecrementByIntervalOnce())
	{
		DebugRenderOptions options;
		options.m_startColor = Rgba::DARK_GREEN;
		options.m_endColor = Rgba::RED;
		options.m_lifetime = 4.f;
		
		DebugRenderSystem::DrawPoint(transform.position, options, 0.2f);
	}

	// Debugging - test raycast
	RaycastHit_t rayhit = Game::GetMap()->Raycast(m_camera->GetPosition(), m_camera->GetForwardVector(), Map::MAX_RAYCAST_DISTANCE);

	if (rayhit.hit)
	{
		DebugRenderSystem::DrawUVSphere(rayhit.position, 0.f);
	}
	else
	{
		DebugRenderSystem::Draw2DText("No hit", Window::GetInstance()->GetWindowBounds(), 0.f);
	}

	SetTarget(true, rayhit.position);
	Tank::Update(deltaTime);
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
	Vector3 rotation = Vector3(rotationOffset.x, rotationOffset.y, 0.f) * CAMERA_ROTATION_SPEED * deltaTime;

	m_camera->RotateHorizontally(-rotation.y);
	m_camera->RotateVertically(-rotation.x);

	float wheelDelta = mouse.GetMouseWheelDelta();
	if (wheelDelta != 0.f)
	{
		m_camera->MoveAlongRadius(wheelDelta * 10.f);
	}

	if (mouse.WasButtonJustPressed(MOUSEBUTTON_RIGHT))
	{
		m_lookAtTarget = false;
	}

	if (mouse.WasButtonJustReleased(MOUSEBUTTON_RIGHT))
	{
		m_lookAtTarget = true;
	}
}


//-----------------------------------------------------------------------------------------------
// Updates the player's transform based on keyboard input
//
void Player::UpdatePositionOnInput(float deltaTime)
{
	InputSystem* input = InputSystem::GetInstance();

	// Rotating the player
	float inputRotation = 0.f;

	if (input->IsKeyPressed('A')) { inputRotation -= 1.f; }		// Turn left
	if (input->IsKeyPressed('D')) { inputRotation += 1.f; }		// Turn right

	transform.Rotate(Vector3(0.f, inputRotation * TANK_ROTATION_SPEED * deltaTime, 0.f));

	// Translating the Player
	Vector3 inputOffset = Vector3::ZERO;
	if (input->IsKeyPressed('W'))								{ inputOffset.z += 1.f; }		// Forward
	if (input->IsKeyPressed('S'))								{ inputOffset.z -= 1.f; }		// Back

	if (inputOffset == Vector3::ZERO)
	{
		return;
	}

	Vector3 worldForward = transform.GetWorldForward();
	worldForward.y = 0;
	worldForward.NormalizeAndGetLength();

	Vector3 forwardTranslation = inputOffset.z * worldForward * (1.5f * TANK_TRANSLATION_SPEED * deltaTime);

	transform.TranslateWorld(forwardTranslation);
}
