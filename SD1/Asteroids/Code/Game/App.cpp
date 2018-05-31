/************************************************************************/
/* Project: SD1 C27 A2 - Asteroids
/* File: App.cpp
/* Author: Andrew Chase
/* Date: September 3rd, 2017
/* Bugs: None
/* Description: Handles communication between the engine and the game
/************************************************************************/
#include "Game/App.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/Time.hpp"


//-----------------------------------------------------------------------------------------------
// Default constructor - creates the Game and initializes it to the initial game state
//
App::App()
{
	g_theRenderer = new Renderer();

	g_theInputSystem = new InputSystem();

	m_theGame = new Game();
	m_theGame->SetupInitialGameState();
}


//-----------------------------------------------------------------------------------------------
// Destructor that deletes the Game instance
//
App::~App()
{
	delete m_theGame;
	m_theGame = nullptr;

	delete g_theInputSystem;
	g_theInputSystem = nullptr;

	delete g_theRenderer;
	g_theRenderer = nullptr;
}


//-----------------------------------------------------------------------------------------------
// Calls begin and end frame on the Render and InputSystem, as well as 
//
void App::RunFrame()
{
	g_theRenderer->BeginFrame();
	g_theInputSystem->BeginFrame();

	Update();
	Render();

	g_theInputSystem->EndFrame();
	g_theRenderer->EndFrame();
}


//-----------------------------------------------------------------------------------------------
// Function called every frame to update the game's time-based calculations
//
void App::Update()
{
	// Setting up deltaTime
	static double timeLastUpdate = GetCurrentTimeSeconds();
	double timeNow = GetCurrentTimeSeconds();
	float deltaTime = static_cast<float>(timeNow - timeLastUpdate);

	// Updated our past frame time variable for the next frame's computation
	timeLastUpdate = timeNow;

	if (g_theInputSystem->WasKeyJustPressed('P') || g_theInputSystem->GetController(0).WasButtonJustPressed(XBOX_BUTTON_BACK))
	{
		m_isPaused = !m_isPaused;
	}

	// Set the coordinate system here in case the App wants to do drawing of its own
	g_theRenderer->SetOrtho(AABB2(0.f, 0.f, 1000.f, 1000.f));
	g_theRenderer->ClearBackBuffer(0.f, 0.f, 0.f, 1.f);

	// If T is pressed, make everything update at 1/10th the magnitude
	if (g_theInputSystem->IsKeyPressed('T')) 
	{
		deltaTime *= 0.1f;
	}

	// Only update the game if it is not paused
	if (!m_isPaused)
	{
		m_theGame->Update(deltaTime);
	}

	// Check for quitting
	if (g_theInputSystem->IsKeyPressed(InputSystem::KEYBOARD_ESCAPE))
	{
		Quit();
	}
}


//-----------------------------------------------------------------------------------------------
// Tells the Game object to render all Entities
//
void App::Render() const
{
	m_theGame->Render();
}


//-----------------------------------------------------------------------------------------------
// Accessor used by Main_Win32 to determine when the app was closed
//
bool App::IsQuitting() const
{
	return m_isQuitting;
}


//-----------------------------------------------------------------------------------------------
// Accessor used by Main_Win32 to tell if the App has paused time
//
bool App::IsPaused() const
{
	return m_isPaused;
}


//-----------------------------------------------------------------------------------------------
// Toggles the App's paused state between playing and being paused
//
void App::TogglePaused()	
{
	m_isPaused = !m_isPaused;
}


//-----------------------------------------------------------------------------------------------
// Sets the App's quit bool to true, so Main_Win32 will destroy it
//
void App::Quit()
{
	m_isQuitting = true;
}
