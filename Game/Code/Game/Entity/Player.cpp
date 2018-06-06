#include "Game/Entity/Player.hpp"
#include "Game/Framework/Game.hpp"
#include "Game/Environment/Map.hpp"
#include "Engine/Assets/AssetDB.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Rendering/Core/Renderer.hpp"
#include "Engine/Rendering/Core/OrbitCamera.hpp"

// Constants
const float Player::CAMERA_ROTATION_SPEED = 45.f;
const float Player::CAMERA_TRANSLATION_SPEED = 10.f;

Player::Player()
{
	Renderer* renderer = Renderer::GetInstance();

	m_camera = new OrbitCamera();
	m_camera->SetColorTarget(renderer->GetDefaultColorTarget());
	m_camera->SetDepthTarget(renderer->GetDefaultDepthTarget());
	m_camera->SetTarget(transform.position);
	m_camera->SetRadius(10.f);
	m_camera->SetAzimuth(45.f);
	m_camera->SetRadiusLimits(5.f, 25.f);
	m_camera->SetAzimuthLimits(15.f, 75.f);

	m_renderable = Renderable(transform.GetModelMatrix(), AssetDB::CreateOrGetMeshGroup("Miku.obj"), AssetDB::CreateOrGetSharedMaterial("Miku_Base"));
}

Player::~Player()
{

}

void Player::ProcessInput()
{
	float deltaTime = Game::GetDeltaTime();

	UpdatePositionOnInput(deltaTime);
	UpdateCameraOnInput(deltaTime);

	m_camera->SetTarget(transform.position);
}

void Player::Update(float deltaTime)
{
	GameObject::Update(deltaTime);

	UpdateHeightOnMap();
}

Camera* Player::GetCamera() const
{
	return m_camera;
}

void Player::UpdateCameraOnInput(float deltaTime)
{
	// Rotating the camera
	Mouse& mouse = InputSystem::GetMouse();
	IntVector2 mouseDelta = mouse.GetMouseDelta();

	Vector2 rotationOffset = Vector2((float) mouseDelta.y, (float) mouseDelta.x) * 0.12f;
	Vector3 rotation = Vector3(rotationOffset.x * CAMERA_ROTATION_SPEED * deltaTime, rotationOffset.y * CAMERA_ROTATION_SPEED * deltaTime, 0.f);

	m_camera->RotateHorizontally(rotation.y);
	m_camera->RotateVertically(rotation.x);
}

void Player::UpdatePositionOnInput(float deltaTime)
{
	InputSystem* input = InputSystem::GetInstance();

	// Translating the camera
	Vector3 translationOffset = Vector3::ZERO;
	if (input->IsKeyPressed('W'))								{ translationOffset.z += 1.f; }		// Forward
	if (input->IsKeyPressed('S'))								{ translationOffset.z -= 1.f; }		// Left
	if (input->IsKeyPressed('A'))								{ translationOffset.x -= 1.f; }		// Back
	if (input->IsKeyPressed('D'))								{ translationOffset.x += 1.f; }		// Right
	if (input->IsKeyPressed(InputSystem::KEYBOARD_SPACEBAR))	{ translationOffset.y += 1.f; }		// Up
	if (input->IsKeyPressed('X'))								{ translationOffset.y -= 1.f; }		// Down

	translationOffset *= deltaTime;

	transform.TranslateLocal(translationOffset);
}

void Player::UpdateHeightOnMap()
{
	Map* map = Game::GetMap();
	float height = map->GetHeightAtPosition(transform.position);
	transform.position.y = height;
}

