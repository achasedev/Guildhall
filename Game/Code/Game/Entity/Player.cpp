/************************************************************************/
/* File: Player.cpp
/* Author: Andrew Chase
/* Date: June 5th, 2018
/* Description: Implementation of the Player class
/************************************************************************/
#include "Engine/Core/Window.hpp"
#include "Game/Entity/Player.hpp"
#include "Game/Framework/Game.hpp"
#include "Game/Environment/Map.hpp"
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
const Vector3	Player::CAMERA_TARGET_OFFSET = Vector3(0.f, 2.f, 0.f);
const float		Player::CAMERA_ROTATION_SPEED = 45.f;
const float		Player::PLAYER_ROTATION_SPEED = 180.f;
const float		Player::PLAYER_TRANSLATION_SPEED = 10.f;


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
	m_camera->SetTarget(transform.position + CAMERA_TARGET_OFFSET);
	m_camera->SetRadiusLimits(5.f, 25.f);
	m_camera->SetRadius(10.f);
	m_camera->SetAzimuthLimits(5.f, 135.f);
	m_camera->SetAzimuth(45.f);
	m_camera->SetProjectionPerspective(90.f, 0.1f, 1000.f);

	// Set up the renderable
	MeshGroup* mikuMesh = AssetDB::CreateOrGetMeshGroup("Data/Models/Miku.obj");
	Material* baseMaterial = AssetDB::CreateOrGetSharedMaterial("Data/Materials/Miku_Base.material");
	Material* quadMaterial = AssetDB::CreateOrGetSharedMaterial("Data/Materials/Miku_Quad.material");
	Material* detailMaterial = AssetDB::CreateOrGetSharedMaterial("Data/Materials/Miku_Detail.material");

	RenderableDraw_t draw;
	m_renderable = new Renderable();

	draw.sharedMaterial = quadMaterial;
	draw.mesh = mikuMesh->GetMesh(0);
	m_renderable->AddDraw(draw);

	draw.sharedMaterial = baseMaterial;
	draw.mesh = mikuMesh->GetMesh(1);
	m_renderable->AddDraw(draw);
	
	draw.sharedMaterial = detailMaterial;
	draw.mesh = mikuMesh->GetMesh(2);
	m_renderable->AddDraw(draw);

	draw.sharedMaterial = baseMaterial;
	draw.mesh = mikuMesh->GetMesh(3);
	m_renderable->AddDraw(draw);


	//m_renderable = AssetDB::LoadFileWithAssimp("Data/Models/Gage/Gage.fbx");
	//Game::GetRenderScene()->AddRenderable(m_renderable);
	
	transform.position = Vector3::ZERO;
	m_stopwatch = new Stopwatch(Game::GetGameClock());
	m_stopwatch->SetInterval(0.5f);
	m_renderable->AddInstanceMatrix(transform.GetModelMatrix());
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
}


//-----------------------------------------------------------------------------------------------
// Sets the actor's position to match the height of th map and spawns the breadcrumbs
//
void Player::Update(float deltaTime)
{
	GameObject::Update(deltaTime);

	UpdateHeightOnMap();
	UpdateOrientationWithNormal();

	m_renderable->SetInstanceMatrix(0, transform.GetModelMatrix());

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
	RaycastHit_t rayhit = Game::GetMap()->Raycast(m_camera->GetPosition(), m_camera->GetForwardVector());

	if (rayhit.hit)
	{
		DebugRenderSystem::DrawUVSphere(rayhit.position, 0.f);
	}
	else
	{
		//DebugRenderSystem::Draw2DQuad(AABB2(Vector2(300.f, 300.f), Vector2(500.f, 500.f)), Rgba::WHITE, 0.f);
		DebugRenderSystem::Draw2DText("No hit", Window::GetInstance()->GetWindowBounds(), 0.f);
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
	if (mouse.IsButtonPressed(MOUSEBUTTON_LEFT) || mouse.IsButtonPressed(MOUSEBUTTON_RIGHT))
	{
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
	}
}


//-----------------------------------------------------------------------------------------------
// Updates the player's transform based on keyboard input
//
void Player::UpdatePositionOnInput(float deltaTime)
{
	InputSystem* input = InputSystem::GetInstance();

	// Rotating the palyer
	float inputRotation = 0.f;

	if (input->IsKeyPressed('A')) { inputRotation -= 1.f; }		// Turn left
	if (input->IsKeyPressed('D')) { inputRotation += 1.f; }		// Turn right

	transform.Rotate(Vector3(0.f, inputRotation * PLAYER_ROTATION_SPEED * deltaTime, 0.f));

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

	Vector3 forwardTranslation = inputOffset.z * worldForward;

	Vector3 worldRight = transform.GetWorldRight();
	worldRight.y = 0;
	worldRight.NormalizeAndGetLength();

	Vector3 rightTranslation = inputOffset.x * worldRight;
	Vector3 worldTranslation = forwardTranslation + rightTranslation;

	worldTranslation.NormalizeAndGetLength();
	worldTranslation *= (PLAYER_TRANSLATION_SPEED * deltaTime);

	transform.TranslateWorld(worldTranslation);
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


//-----------------------------------------------------------------------------------------------
// Finds the normal of the terrain at the player's postion and sets the player to be oriented with that normal
//
void Player::UpdateOrientationWithNormal()
{
	Map* map = Game::GetMap();

	Vector3 normal = map->GetNormalAtPosition(transform.position);
	Vector3 right = CrossProduct(normal, transform.GetWorldForward());
	right.NormalizeAndGetLength();

	Vector3 newForward = CrossProduct(right, normal);
	newForward.NormalizeAndGetLength();

	Matrix44 newModel = Matrix44(right, normal, newForward, transform.position);
	transform.SetModelMatrix(newModel);
}
