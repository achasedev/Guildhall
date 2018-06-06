#include "Game/Entity/Player.hpp"
#include "Game/Framework/Game.hpp"
#include "Game/Environment/Map.hpp"
#include "Engine/Assets/AssetDB.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Rendering/Core/Renderer.hpp"
#include "Engine/Rendering/Core/OrbitCamera.hpp"

// Constants
const float Player::PLAYER_ROTATION_SPEED = 45.f;
const float Player::PLAYER_TRANSLATION_SPEED = 10.f;

Player::Player()
{
	Renderer* renderer = Renderer::GetInstance();

	m_camera = new OrbitCamera();
	m_camera->SetColorTarget(renderer->GetDefaultColorTarget());
	m_camera->SetDepthTarget(renderer->GetDefaultDepthTarget());
	m_camera->SetTarget(transform.position);
	m_camera->SetRadiusLimits(5.f, 25.f);
	m_camera->SetRadius(10.f);
	m_camera->SetAzimuthLimits(15.f, 75.f);
	m_camera->SetAzimuth(45.f);
	m_camera->SetProjectionPerspective(90.f, 0.1f, 1000.f);

	Material* baseMaterial = AssetDB::CreateOrGetSharedMaterial("Miku_Base");
	Material* quadMaterial = AssetDB::CreateOrGetSharedMaterial("Miku_Quad");
	Material* detailMaterial = AssetDB::CreateOrGetSharedMaterial("Miku_Detail");

	m_renderable = Renderable(transform.GetModelMatrix(), AssetDB::CreateOrGetMeshGroup("Miku.obj"), baseMaterial);
	m_renderable.SetSharedMaterial(quadMaterial, 0);
	m_renderable.SetSharedMaterial(detailMaterial, 2);

	transform.position = Vector3::ZERO;
}

Player::~Player()
{

}

void Player::ProcessInput()
{
	float deltaTime = Game::GetDeltaTime();

	UpdatePositionOnInput(deltaTime);
	UpdateCameraOnInput(deltaTime);

	Mouse& mouse = InputSystem::GetMouse();
	if (mouse.IsButtonPressed(MOUSEBUTTON_RIGHT))
	{
		transform.rotation.y = m_camera->GetRotation().y;
	}

	m_camera->SetTarget(transform.position);
}

void Player::Update(float deltaTime)
{
	GameObject::Update(deltaTime);

	UpdateHeightOnMap();

	m_renderable.SetModelMatrix(transform.GetModelMatrix(), 0);
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
	Vector3 rotation = Vector3(rotationOffset.x * PLAYER_ROTATION_SPEED * deltaTime, rotationOffset.y * PLAYER_ROTATION_SPEED * deltaTime, 0.f);

	m_camera->RotateHorizontally(-rotation.y);
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

	translationOffset *= PLAYER_TRANSLATION_SPEED * deltaTime;

	transform.TranslateLocal(translationOffset);
}

void Player::UpdateHeightOnMap()
{
	Map* map = Game::GetMap();
	float height = map->GetHeightAtPosition(transform.position);
	transform.position.y = height;
}

