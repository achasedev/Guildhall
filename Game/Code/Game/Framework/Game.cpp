/************************************************************************/
/* File: Game.cpp
/* Author: Andrew Chase
/* Date: March 24th, 2018
/* Description: Game class for general gameplay management
/************************************************************************/
#include "Game/Framework/Game.hpp"
#include "Game/Environment/World.hpp"
#include "Game/Framework/GameCommon.hpp"
#include "Game/Framework/GameCamera.hpp"
#include "Game/Environment/BlockType.hpp"
#include "Engine/Core/Window.hpp"
#include "Engine/Assets/AssetDB.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Time/Clock.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Rendering/Core/Camera.hpp"
#include "Engine/Rendering/Core/Renderer.hpp"
#include "Engine/Core/Utility/Blackboard.hpp"
#include "Engine/Rendering/Materials/Material.hpp"
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
	m_gameCamera = new GameCamera();
	m_gameCamera->SetColorTarget(renderer->GetDefaultColorTarget());
	m_gameCamera->SetDepthTarget(renderer->GetDefaultDepthTarget());
	m_gameCamera->SetProjectionPerspective(45.f, 0.1f, 1000.f);
	m_gameCamera->LookAt(Vector3(-32.f, -16.f, 48.f), Vector3::ZERO, Vector3::Z_AXIS);

	DebugRenderSystem::SetWorldCamera(m_gameCamera);

	// World
	m_world = new World();
	
	// Create the block types and load the texture
	BlockType::InitializeTypes();

	PopulateGameConfigBlackboard();
}


//-----------------------------------------------------------------------------------------------
// Loads GameConfig.xml and sets the values in the black board from it
//
void Game::PopulateGameConfigBlackboard()
{
	m_gameConfigBlackboard = new Blackboard();

	XMLDocument document;
	XMLError error = document.LoadFile("Data/Config/GameConfig.xml");

	GUARANTEE_RECOVERABLE(error == tinyxml2::XML_SUCCESS, "Couldn't open GameConfig.xml");

	const XMLElement* rootElement = document.RootElement();
	const XMLElement* chunkElement = rootElement->FirstChildElement("Chunk");
	m_gameConfigBlackboard->PopulateFromXmlElementAttributes(*chunkElement);
}


//-----------------------------------------------------------------------------------------------
// Destructor - clean up any allocated members (private)
//
Game::~Game()
{
	delete m_world;
	m_world = nullptr;

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

	DebugRenderOptions options;
	options.m_lifetime = 99999.f;
	options.m_renderMode = DEBUG_RENDER_XRAY;

	DebugRenderSystem::DrawBasis(Matrix44::IDENTITY, options);
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
	m_gameCamera->ProcessInput();
	m_world->ProcessInput();
}


//-----------------------------------------------------------------------------------------------
// Update the movement variables of every entity in the world, as well as update the game state
// based on the input this frame
//
void Game::Update()
{
	m_world->Update();
}



//-----------------------------------------------------------------------------------------------
// Draws to screen
//
void Game::Render() const
{
	// Render the chunks
	Renderer* renderer = Renderer::GetInstance();
	renderer->SetCurrentCamera(m_gameCamera);
	m_world->Render();

	// Debug draws for camera
	Window* window = Window::GetInstance();
	AABB2 windowBounds = window->GetWindowBounds();
	
	Vector3 position = m_gameCamera->GetPosition();
	Vector3 rotation = m_gameCamera->GetRotation();
	IntVector2 chunkContainingCamera = m_world->GetChunkCoordsForChunkThatContainsPosition(position);

	std::string text = Stringf("Camera Pos : (%.2f, %.2f, %.2f)\nCamera Rot : (%.2f, %.2f, %.2f)\nChunk Containing : (%i, %i)",
		position.x, position.y, position.z, rotation.x, rotation.y, rotation.z, chunkContainingCamera.x, chunkContainingCamera.y);

	DebugRenderSystem::Draw2DText(text, windowBounds, 0.f, Rgba::DARK_GREEN, 20.f);

	// Screen reticle
	renderer->SetCurrentCamera(renderer->GetUICamera());

	float size = 100.f;
	AABB2 uiBounds = renderer->GetUIBounds();
	Vector2 center = uiBounds.GetCenter();

	AABB2 crosshairBounds = AABB2(center - Vector2(0.5f * size), center + Vector2(0.5f * size));

	Material material;
	material.SetShader(AssetDB::GetShader("UI"));
	material.SetDiffuse(AssetDB::CreateOrGetTexture("Data/Images/Crosshair.png"));

	renderer->Draw2DQuad(crosshairBounds, AABB2::UNIT_SQUARE_OFFCENTER, Rgba::LIGHT_BLUE, &material);
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
GameCamera* Game::GetGameCamera()
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
// Returns the world
//
World* Game::GetWorld()
{
	return s_instance->m_world;
}


//-----------------------------------------------------------------------------------------------
// Returns the blackboard used to set global game configurations
//
Blackboard* Game::GetGameConfigBlackboard()
{
	return s_instance->m_gameConfigBlackboard;
}


//-----------------------------------------------------------------------------------------------
// Returns the singleton Game instance
//
Game* Game::GetInstance()
{
	return s_instance;
}
