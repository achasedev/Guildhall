/************************************************************************/
/* File: Game.cpp
/* Author: Andrew Chase
/* Date: March 24th, 2018
/* Description: Game class for general gameplay management
/************************************************************************/
#include "Game/Framework/Game.hpp"
#include "Game/Framework/GameCommon.hpp"
#include "Engine/Core/Window.hpp"
#include "Engine/Core/Time/Clock.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Rendering/Core/Camera.hpp"
#include "Engine/Rendering/Core/Renderer.hpp"
#include "Engine/Rendering/DebugRendering/DebugRenderSystem.hpp"

// The singleton instance
Game* Game::s_instance = nullptr;

//-----------------------------------------------------------------------------------------------
// Default constructor, initialize any game members here (private)
//
Game::Game()
{
	// Clock
	m_gameClock = new Clock(Clock::GetMasterClock());
	m_gameClock->SetMaxDeltaTime((1.f / 30.f)); // Limit the time step to be no more than 1/30th a frame

	// Camera
	Renderer* renderer = Renderer::GetInstance();
	m_gameCamera = new Camera();
	m_gameCamera->SetColorTarget(renderer->GetDefaultColorTarget());
	m_gameCamera->SetDepthTarget(renderer->GetDefaultDepthTarget());
	m_gameCamera->SetProjectionPerspective(45.f, 0.1f, 1000.f);
	m_gameCamera->LookAt(Vector3(10.f, 10.f, 10.f), Vector3::ZERO, Vector3::Z_AXIS);

	DebugRenderSystem::SetWorldCamera(m_gameCamera);

	m_child.SetParentTransform(&m_parent);
}


//-----------------------------------------------------------------------------------------------
// Destructor - clean up any allocated members (private)
//
Game::~Game()
{
	delete m_gameCamera;
	m_gameCamera = nullptr;
}


//-----------------------------------------------------------------------------------------------
// Constructs the singleton game instance
//
void Game::Initialize()
{
	GUARANTEE_OR_DIE(s_instance == nullptr, "Error: Game::Initialize called when a Game instance already exists.");
	s_instance = new Game();

	// Set the game clock on the Renderer
	Renderer::GetInstance()->SetRendererGameClock(s_instance->m_gameClock);

	// Set up the mouse for FPS controls (Free camera)
	Mouse& mouse = InputSystem::GetMouse();

	mouse.ShowMouseCursor(false);
	mouse.LockCursorToClient(true);
	mouse.SetCursorMode(CURSORMODE_RELATIVE);

	DebugRenderSystem::DrawBasis(Vector3::ZERO, Vector3::ZERO, 999.f, 2.f);
}


//-----------------------------------------------------------------------------------------------
// Deletes the singleton instance
//
void Game::ShutDown()
{
	delete s_instance;
	s_instance = nullptr;
}


//-----------------------------------------------------------------------------------------------
// Checks for input received this frame and updates states accordingly
//
void Game::ProcessInput()
{
	InputSystem* input = InputSystem::GetInstance();
	Mouse& mouse = InputSystem::GetMouse();

	float deltaTime = Game::GetDeltaTime();
	IntVector2 delta = mouse.GetMouseDelta();

	// Translating the camera
	Vector3 translationOffset = Vector3::ZERO;
	if (input->IsKeyPressed('W')) { translationOffset.x += 1.f; }		// Forward
	if (input->IsKeyPressed('A')) { translationOffset.y += 1.f; }		// Left
	if (input->IsKeyPressed('S')) { translationOffset.x -= 1.f; }		// Back
	if (input->IsKeyPressed('D')) { translationOffset.y -= 1.f; }		// Right
	if (input->IsKeyPressed(InputSystem::KEYBOARD_SPACEBAR)) { translationOffset.z += 1.f; }		// Up
	if (input->IsKeyPressed('X')) { translationOffset.z -= 1.f; }		// Down

	if (input->IsKeyPressed(InputSystem::KEYBOARD_SHIFT))
	{
		translationOffset *= 50.f;
	}

	translationOffset *= CAMERA_TRANSLATION_SPEED * deltaTime;

	m_gameCamera->TranslateLocal(translationOffset);

	Vector2 rotationOffset = Vector2((float)delta.y, (float)delta.x) * 0.12f;
	Vector3 rotation = Vector3(0.f, rotationOffset.x * CAMERA_ROTATION_SPEED * deltaTime, -1.0f * rotationOffset.y * CAMERA_ROTATION_SPEED * deltaTime);

	m_gameCamera->Rotate(rotation);
}

#include "Engine/Core/DeveloperConsole/DevConsole.hpp"
#include "Engine/Math/MathUtils.hpp"
//-----------------------------------------------------------------------------------------------
// Update the movement variables of every entity in the world, as well as update the game state
// based on the input this frame
//
void Game::Update()
{
	float time = Game::GetGameClock()->GetTotalSeconds();
	m_parent.SetPosition(10.f * Vector3(CosDegrees(30.f * time), SinDegrees(30.f * time), 0.f));
	m_parent.SetRotation(Vector3(0.f, 0.f, Atan2Degrees(m_parent.position.y, m_parent.position.x)));

	m_child.SetPosition(3.f * Vector3(0.f, CosDegrees(30.f * time), SinDegrees(30.f * time)));
	m_child.SetRotation(Vector3(Atan2Degrees(m_child.position.z, m_child.position.y), 0.f, 0.f));
}

#include "Engine/Assets/AssetDB.hpp"
#include "Engine/Rendering/Materials/MaterialInstance.hpp"
//-----------------------------------------------------------------------------------------------
// Draws to screen
//
void Game::Render() const
{
	Window* window = Window::GetInstance();
	AABB2 bounds = window->GetWindowBounds();
	
	Vector3 position = m_gameCamera->GetPosition();
	Vector3 rotation = m_gameCamera->GetRotation();

	std::string text = Stringf("Camera Pos : (%.2f, %.2f, %.2f)\nCamera Rot : (%.2f, %.2f, %.2f)",
		position.x, position.y, position.z, rotation.x, rotation.y, rotation.z);

	DebugRenderSystem::Draw2DText(text, bounds, 0.f);
	
	renderable.ClearAll();
	renderable.AddInstanceMatrix(Matrix44::IDENTITY);

	RenderableDraw_t draw;
	draw.drawMatrix = m_parent.GetWorldMatrix();
	draw.mesh = AssetDB::CreateOrGetMesh("Cube");
	draw.materialInstance = AssetDB::CreateMaterialInstance("Default_Opaque");
	draw.materialInstance->SetDiffuse(AssetDB::CreateOrGetTexture("Data/Images/Debug/Debug.png"));
	Shader* shader = draw.materialInstance->GetEditableShader();
	//shader->SetCullMode(CULL_MODE_NONE);
	//shader->SetWindOrder(WIND_CLOCKWISE);

	renderable.AddDraw(draw);
	
	draw.drawMatrix = m_child.GetWorldMatrix();
	draw.materialInstance = AssetDB::CreateMaterialInstance("Default_Opaque");
	draw.materialInstance->SetDiffuse(AssetDB::CreateOrGetTexture("Data/Images/Debug/Debug.png"));
	Shader* shader2 = draw.materialInstance->GetEditableShader();

	renderable.AddDraw(draw);

	Renderer* renderer = Renderer::GetInstance();
	renderer->SetCurrentCamera(m_gameCamera);
	renderer->DrawRenderable(&renderable);
}


//-----------------------------------------------------------------------------------------------
// Returns the game clock
//
Clock* Game::GetGameClock()
{
	return s_instance->m_gameClock;
}


//-----------------------------------------------------------------------------------------------
// Returns the camera used to render game elements
//
Camera* Game::GetGameCamera()
{
	return s_instance->m_gameCamera;
}


//-----------------------------------------------------------------------------------------------
// Returns the frame time for the game clock
//
float Game::GetDeltaTime()
{
	return s_instance->m_gameClock->GetDeltaTime();
}


//-----------------------------------------------------------------------------------------------
// Returns the singleton Game instance
//
Game* Game::GetInstance()
{
	return s_instance;
}
