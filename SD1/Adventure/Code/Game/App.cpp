/************************************************************************/
/* Project: Protogame2D
/* File: App.cpp
/* Author: Andrew Chase
/* Date: September 23rd, 2017
/* Bugs: None
/* Description: Handles communication between the engine and the game
/************************************************************************/
#include "Game/App.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/Blackboard.hpp"
//-----------------------------------------------------------------------------------------------
// Default constructor - creates the Game and initializes it to the initial game state
//
App::App()
{
	SetupGameConfigBlackboard();

	g_theRenderer = new Renderer();

	g_theInputSystem = new InputSystem();

	g_theAudioSystem = new AudioSystem();

	g_screenBounds = new AABB2(Vector2::ZERO, Vector2(16.f, 9.f));

	LoadResources();

	g_theGame = new Game();
}


//-----------------------------------------------------------------------------------------------
// Destructor that deletes the Game instance
//
App::~App()
{
	delete g_theAudioSystem;
	g_theAudioSystem = nullptr;

	delete g_theGame;
	g_theGame = nullptr;

	delete g_screenBounds;
	g_screenBounds = nullptr;

	delete g_theInputSystem;
	g_theInputSystem = nullptr;

	delete g_theRenderer;
	g_theRenderer = nullptr;

	delete g_gameConfigBlackboard;
	g_gameConfigBlackboard = nullptr;
}


//-----------------------------------------------------------------------------------------------
// Calls begin and end frame on the Render and InputSystem, as well as 
//
void App::RunFrame()
{
	g_theRenderer->BeginFrame();
	g_theInputSystem->BeginFrame();
	g_theAudioSystem->BeginFrame();

	Update();
	Render();

	g_theAudioSystem->EndFrame();
	g_theInputSystem->EndFrame();
	g_theRenderer->EndFrame();
}


//-----------------------------------------------------------------------------------------------
// Function called every frame to update the game's time-based calculations
//
void App::Update()
{
	if (g_theInputSystem->WasKeyJustPressed(InputSystem::KEYBOARD_F3))
	{
		g_inDebugMode = !g_inDebugMode;
	}

	if (g_theInputSystem->WasKeyJustPressed(InputSystem::KEYBOARD_F2))
	{
		m_fastTime = !m_fastTime;
	}

	// Get deltaTime
	float deltaTime = GetDeltaTime();
	
	if (m_fastTime)
	{
		deltaTime *= 5.f;
	}
	
	g_theGame->Update(deltaTime);
}


//-----------------------------------------------------------------------------------------------
// Tells the Game object to render all Entities
//
void App::Render() const
{
	// Set the coordinate system here in case the App wants to do drawing of its own
	g_theRenderer->SetOrtho(AABB2(0.f, 0.f, 24.f, 13.5f));
	g_theRenderer->ClearScreen(Rgba::BLACK);

	g_theGame->Render();

	// Draw here to draw on top of game
}


//-----------------------------------------------------------------------------------------------
// Calculates the time since the last frame, and returns it
//
float App::GetDeltaTime() const
{
	// Setting up deltaTime
	static double timeLastUpdate = GetCurrentTimeSeconds();
	double timeNow = GetCurrentTimeSeconds();
	float deltaTime = static_cast<float>(timeNow - timeLastUpdate);

	// Min frame rate lock, to ensure when debugging breakpoints don't
	// cause excessive delta times
	float maxDeltaTime = (1.f / static_cast<float>(MIN_FRAMERATE));
	if (deltaTime > maxDeltaTime)
	{
		deltaTime = maxDeltaTime;
	}

	// Updated our past frame time variable for the next frame's computation
	timeLastUpdate = timeNow;

	if (g_theInputSystem->IsKeyPressed('T'))
	{
		deltaTime *= 0.1f;
	}

	return deltaTime;
}


//-----------------------------------------------------------------------------------------------
// Sets up the global blackboard for game settings and external information
//
void App::SetupGameConfigBlackboard()
{
	tinyxml2::XMLDocument configDocument;
	configDocument.LoadFile("Data/GameConfig.xml");
	g_gameConfigBlackboard = new Blackboard();
	tinyxml2::XMLElement* currElement = configDocument.RootElement();

	while (currElement != nullptr)
	{
		g_gameConfigBlackboard->PopulateFromXmlElementAttributes(*currElement);
		currElement = currElement->NextSiblingElement();
	}
}


//-----------------------------------------------------------------------------------------------
// Loads textures and audio from disk
//
void App::LoadResources() const
{
	g_theAudioSystem->CreateOrGetSound(ATTRACT_MUSIC);
	g_theAudioSystem->CreateOrGetSound(GAMEOVER_MUSIC);
	g_theAudioSystem->CreateOrGetSound(VICTORY_MUSIC);
	g_theAudioSystem->CreateOrGetSound(PLAYING_MUSIC);
}


//-----------------------------------------------------------------------------------------------
// Accessor used by Main_Win32 to determine when the app was closed
//
bool App::IsQuitting() const
{
	return m_isQuitting;
}


//-----------------------------------------------------------------------------------------------
// Sets the App's quit bool to true, so Main_Win32 will destroy it
//
void App::Quit()
{
	m_isQuitting = true;
}
