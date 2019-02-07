/************************************************************************/
/* File: Game.cpp
/* Author: Andrew Chase
/* Date: March 24th, 2018
/* Description: Game class for general gameplay management
/************************************************************************/
#include "Game/Framework/Game.hpp"
#include "Game/Audio/FFTSystem.hpp"
#include "Game/Framework/GameCommon.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/Time/Clock.hpp"
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
	m_gameCamera->LookAt(Vector3(0.f, 5.0, -5.f), Vector3::ZERO);

	DebugRenderSystem::SetWorldCamera(m_gameCamera);

	// FFT System
	m_fftSystem = new FFTSystem();

	SoundID sound = m_fftSystem->CreateOrGetSound("Data/Audio/Music/200hz.mp3");
// 	SoundID sound = AudioSystem::GetInstance()->CreateOrGetSound("Data/Audio/Music/50hz.mp3");
// 	SoundID sound4 = AudioSystem::GetInstance()->CreateOrGetSound("Data/Audio/Music/200hz.mp3");
// 	SoundID sound2 = AudioSystem::GetInstance()->CreateOrGetSound("Data/Audio/Music/1000hz.mp3");
// 	SoundID sound3 = AudioSystem::GetInstance()->CreateOrGetSound("Data/Audio/Music/5000hz.mp3");

	m_fftSystem->PlaySound(sound, true);
// 	AudioSystem::GetInstance()->PlaySound(sound2, true);
// 	AudioSystem::GetInstance()->PlaySound(sound3, true);
// 	AudioSystem::GetInstance()->PlaySound(sound4, true);
}

	
//-----------------------------------------------------------------------------------------------
// Destructor - clean up any allocated members (private)
//
Game::~Game()
{
	delete m_fftSystem;
	m_fftSystem = nullptr;

	delete m_gameCamera;
	m_gameCamera = nullptr;

	delete m_gameClock;
	m_gameClock = nullptr;
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
	m_fftSystem->ProcessInput();
}


//-----------------------------------------------------------------------------------------------
// Update the movement variables of every entity in the world, as well as update the game state
// based on the input this frame
//
void Game::Update()
{
	m_fftSystem->BeginFrame();
}


//-----------------------------------------------------------------------------------------------
// Draws to screen
//
void Game::Render() const
{
	if (m_fftSystem->IsSetToRenderGraph())
	{
		m_fftSystem->Render();
	}
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
