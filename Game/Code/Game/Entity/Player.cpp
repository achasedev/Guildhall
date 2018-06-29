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
#include "Game/Framework/Game.hpp"
#include "Game/Environment/Map.hpp"
#include "Game/Entity/ChargeBullet.hpp"

#include "Engine/Core/Window.hpp"
#include "Engine/Assets/AssetDB.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Time/Clock.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Audio/AudioSystem.hpp"
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
const float		Player::PLAYER_CHARGESHOT_CHARGE_TIME = 3.f;


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
	m_camera->SetRadiusLimits(5.f, 50.f);
	m_camera->SetRadius(15.f);
	m_camera->SetAzimuthLimits(5.f, 135.f);
	m_camera->SetAzimuth(45.f);
	m_camera->SetProjectionPerspective(90.f, 0.1f, 1000.f);

	transform.position = Vector3::ZERO;
	m_stopwatch = new Stopwatch(Game::GetGameClock());
	m_stopwatch->SetInterval(0.5f);

	m_type = ENTITY_PLAYER;

	for (int i = 0; i < 4; ++i)
	{
		m_mikuMeme[i] = new Renderable();

		MeshGroup* group = AssetDB::CreateOrGetMeshGroup("Data/Models/Miku.obj");

		RenderableDraw_t draw;

		Vector3 position;

		if (i == 0)			{ position = Vector3(-2.f, 1.f, 2.f); }
		else if (i == 1)	{ position = Vector3(2.f, 1.f, 2.f); }
		else if (i == 2)	{ position = Vector3(2.f, 1.f, -2.f); }
		else				{ position = Vector3(-2.f, 1.f, -2.f); }

		draw.drawMatrix = Matrix44::MakeModelMatrix(position, Vector3::ZERO, Vector3::ONES);
		draw.mesh = group->GetMesh(0);
		draw.sharedMaterial = AssetDB::CreateOrGetSharedMaterial("Data/Materials/Miku_Quad.material");
		m_mikuMeme[i]->AddDraw(draw);

		draw.mesh = group->GetMesh(1);
		draw.sharedMaterial = AssetDB::CreateOrGetSharedMaterial("Data/Materials/Miku_Base.material");
		m_mikuMeme[i]->AddDraw(draw);

		draw.mesh = group->GetMesh(2);
		draw.sharedMaterial = AssetDB::CreateOrGetSharedMaterial("Data/Materials/Miku_Base.material");
		m_mikuMeme[i]->AddDraw(draw);

		draw.mesh = group->GetMesh(3);
		draw.sharedMaterial = AssetDB::CreateOrGetSharedMaterial("Data/Materials/Miku_Detail.material");
		m_mikuMeme[i]->AddDraw(draw);

		m_mikuMeme[i]->AddInstanceMatrix(transform.GetWorldMatrix());
	}
}


//-----------------------------------------------------------------------------------------------
// Destructor
//
Player::~Player()
{
	Game::GetRenderScene()->RemoveRenderable(m_renderable);

	delete m_stopwatch;
	m_stopwatch = nullptr;

	Game::GetRenderScene()->RemoveCamera(m_camera);
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
	InputSystem* input = InputSystem::GetInstance();

	if (mouse.WasButtonJustPressed(MOUSEBUTTON_LEFT) || mouse.IsButtonPressed(MOUSEBUTTON_LEFT) && input->IsKeyPressed(InputSystem::KEYBOARD_SHIFT))
	{
		ShootCannon();
	}


	if (input->WasKeyJustPressed(InputSystem::KEYBOARD_SHIFT))
	{
		for (int i = 0; i < 4; ++i)
		{
			Game::GetRenderScene()->AddRenderable(m_mikuMeme[i]);	
		}

		m_fireRate = TANK_DEFAULT_FIRERATE * 100.f;
	}

	if (input->WasKeyJustReleased(InputSystem::KEYBOARD_SHIFT))
	{
		for (int i = 0; i < 4; ++i)
		{
			Game::GetRenderScene()->RemoveRenderable(m_mikuMeme[i]);	
		}	

		m_fireRate = TANK_DEFAULT_FIRERATE;
	}

	if (input->WasKeyJustReleased(InputSystem::KEYBOARD_SPACEBAR) && m_chargeShotTimer == PLAYER_CHARGESHOT_CHARGE_TIME)
	{
		ShootChargeShot();
	}

	if (input->IsKeyPressed(InputSystem::KEYBOARD_SPACEBAR))
	{
		float deltaAmount = (input->IsKeyPressed(InputSystem::KEYBOARD_SHIFT) ? deltaTime * 5.f : deltaTime);
		m_chargeShotTimer = ClampFloat(m_chargeShotTimer += deltaAmount, 0.f, PLAYER_CHARGESHOT_CHARGE_TIME);
	}
	else
	{
		float deltaAmount = (input->IsKeyPressed(InputSystem::KEYBOARD_SHIFT) ? deltaTime * 5.f : deltaTime);
		m_chargeShotTimer = ClampFloat(m_chargeShotTimer -= deltaAmount, 0.f, PLAYER_CHARGESHOT_CHARGE_TIME);
	}
}


//-----------------------------------------------------------------------------------------------
// Sets the actor's position to match the height of th map and spawns the breadcrumbs
//
void Player::Update(float deltaTime)
{
	// Debugging - test raycast
	Map* map = Game::GetMap();
	RaycastHit_t rayhit = map->Raycast(m_camera->GetPosition(), m_camera->GetForwardVector(), Map::MAX_RAYCAST_DISTANCE);

	SetTarget(true, rayhit.position);
	Tank::Update(deltaTime);

	for (int i = 0; i < 4; ++i)
	{
		Matrix44 mikuMatrix = transform.GetWorldMatrix() * Matrix44::MakeRotation(Vector3(0.f, Game::GetGameClock()->GetTotalSeconds() * 360.f, 0.f));
		m_mikuMeme[i]->SetInstanceMatrix(0, mikuMatrix);
	}
}


void Player::OnCollisionWithEntity(GameEntity* other)
{
	if (other->GetType() == ENTITY_SWARMER)
	{
		TakeDamage(1);
	}

	Tank::OnCollisionWithEntity(other);
}

void Player::Respawn()
{
	SetMarkedForDelete(false);
	SetHealth(10);
	transform.position = Game::GetMap()->GetPositionAwayFromEnemies();
}

//-----------------------------------------------------------------------------------------------
// Returns the player's camera
//
Camera* Player::GetCamera() const
{
	return m_camera;
}


float Player::GetChargeTimerNormalized() const
{
	return (m_chargeShotTimer / PLAYER_CHARGESHOT_CHARGE_TIME);
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
		m_camera->MoveAlongRadius(-wheelDelta * 5.f);
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

	if (input->IsKeyPressed(InputSystem::KEYBOARD_SHIFT))
	{
		inputRotation *= 3.f;
	}

	transform.Rotate(Vector3(0.f, inputRotation * TANK_ROTATION_SPEED * deltaTime, 0.f));

	// Translating the Player
	Vector3 inputOffset = Vector3::ZERO;
	if (input->IsKeyPressed('W'))								{ inputOffset.z += 1.f; }		// Forward
	if (input->IsKeyPressed('S'))								{ inputOffset.z -= 1.f; }		// Back

	if (input->IsKeyPressed(InputSystem::KEYBOARD_SHIFT))
	{
		inputOffset *= 10.f;
	}

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

void Player::ShootChargeShot()
{
	Matrix44 fireTransform = m_turret->GetCannon()->GetFireTransform().GetWorldMatrix();
	Vector3 position = Matrix44::ExtractTranslation(fireTransform);
	Quaternion rotation = Quaternion::FromEuler(Matrix44::ExtractRotationDegrees(fireTransform));

	ChargeBullet* bullet = new ChargeBullet(position, rotation, m_team);
	Game::GetMap()->AddGameEntity(bullet);

	m_chargeShotTimer = 0.f;

	// Play an audio shot sound
	AudioSystem* audio = AudioSystem::GetInstance();
	audio->PlaySoundFromAudioGroup("player.cannon");
}
