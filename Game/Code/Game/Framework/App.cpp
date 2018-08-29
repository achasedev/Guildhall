/************************************************************************/
/* File: App.cpp
/* Author: Andrew Chase
/* Date: March 24th, 2018
/* Description: Handles communication between the engine and the game
/************************************************************************/
#include "Game/Framework/App.hpp"
#include "Game/Framework/GameCommon.hpp"
#include "Engine/Core/Window.hpp"
#include "Engine/Core/LogSystem.hpp"
#include "Game/Framework/GameCommon.hpp"
#include "Engine/Assets/AssetDB.hpp"
#include "Engine/Networking/Net.hpp"
#include "Engine/Core/Time/Clock.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/Time/Profiler.hpp"
#include "Engine/Core/Utility/Blackboard.hpp"
#include "Engine/Rendering/Core/Renderer.hpp"
#include "Engine/Rendering/Core/RenderScene.hpp"
#include "Engine/Core/DeveloperConsole/Command.hpp"
#include "Engine/Core/DeveloperConsole/DevConsole.hpp"
#include "Engine/Rendering/Thesis/RayTraceRenderer.hpp"
#include "Engine/Rendering/DebugRendering/DebugRenderSystem.hpp"

// For testing the log system
#include "Engine/Core/File.hpp"
#include "Engine/Core/DeveloperConsole/Command.hpp"
#include "Engine/Core/Threading/Threading.hpp"

// Static instance for singleton behavior
App* App::s_instance = nullptr;


//////////////////////////////////////////////////////////////////////////
// Console Commands
//////////////////////////////////////////////////////////////////////////

// Quits the system
void Command_Quit(Command& cmd)
{
	UNUSED(cmd);
	ConsolePrintf("Quitting...");
	App::GetInstance()->Quit();
}


//-----------------------------------------------------------------------------------------------
// Default constructor - creates the Game and initializes it to the initial game state
//
App::App()
	: m_isQuitting(false)
	, m_timeSinceProgramStart(0.f)
{
	ASSERT_OR_DIE(s_instance == nullptr, "Error: App constructor called when a singleton App instance already exists");
}


//-----------------------------------------------------------------------------------------------
// Destructor that deletes the Game instance
//
App::~App()
{
	// Shutdown in reverse order of initialization
	RayTraceRenderer::ShutDown();
	Net::Shutdown();
	Game::ShutDown();
	DebugRenderSystem::Shutdown();
	Command::Shutdown();
	DevConsole::Shutdown();

	AudioSystem::Shutdown();
	InputSystem::Shutdown();
	Renderer::Shutdown();
	Profiler::Shutdown();
	LogSystem::Shutdown();

	delete g_gameConfigBlackboard;
	g_gameConfigBlackboard = nullptr;
}


//-----------------------------------------------------------------------------------------------
// Creates the singleton App instance, and initializes systems
//
void App::Initialize()
{
	if (s_instance == nullptr)
	{
		// To print the time taken
		ProfileScoped sp = ProfileScoped("Engine Startup"); UNUSED(sp);

		// Setting up the App
		s_instance = new App();
		//s_instance->SetupGameConfigBlackboard();
		s_instance->RegisterAppCommands();


		// Construct the Engine Systems
		AssetDB::CreateBuiltInAssets();

		InputSystem::Initialize();

		Renderer::Initialize();
		s_instance->RenderInitScreen();
		LogSystem::Initialize();
		Profiler::Initialize();
		Thread::RegisterConsoleCommands();

		Clock::Initialize();
		AudioSystem::Initialize();
		DevConsole::Initialize();
		Command::Initialize();
		DebugRenderSystem::Initialize();

		// For networking
		Net::Initialize();

		// For ray tracing
		RayTraceRenderer::Initialize();

		// Make the game instance
		Game::Initialize();
	}
}


//-----------------------------------------------------------------------------------------------
// Calls begin and end frame on the Render and InputSystem, as well as 
//
void App::RunFrame()
{
	Clock::GetMasterClock()->BeginFrame();
	Profiler::BeginFrame();
	Renderer::GetInstance()->BeginFrame();
	InputSystem::GetInstance()->BeginFrame();
	AudioSystem::GetInstance()->BeginFrame();

	ProcessInput();

	Update();
	Render();

	AudioSystem::GetInstance()->EndFrame();
	InputSystem::GetInstance()->EndFrame();
	Renderer::GetInstance()->EndFrame();
	Profiler::EndFrame();
}


//-----------------------------------------------------------------------------------------------
// Function called every frame to update the game's time-based calculations
//
void App::Update()
{
	PROFILE_LOG_SCOPE("App::Update");

	// Update the console before game
	if (DevConsole::IsDevConsoleOpen())
	{
		DevConsole::GetInstance()->Update();
	}

	// Game still updates regardless of DevConsole state
	Game::GetInstance()->Update();
}


//-----------------------------------------------------------------------------------------------
// Tells the Game object to render all Entities
//
void App::Render() const
{
	PROFILE_LOG_SCOPE_FUNCTION();

	// Render the game
	Game::GetInstance()->Render();

	// Update and Render debug
	// Updating here in Render to ensure one-frame tasks are cleaned up this frame
	DebugRenderSystem::GetInstance()->UpdateAndRender();

	// Draw the Profiler if it is open
	if (Profiler::IsProfilerOpen())
	{
		Profiler::GetInstance()->Render();
	}

	// Draw the DevConsole if it is open
	if (DevConsole::IsDevConsoleOpen())
	{
		DevConsole::GetInstance()->Render();
	}
}


//-----------------------------------------------------------------------------------------------
// Draws an initialization screen while the engine systems are being created
//
void App::RenderInitScreen() const
{
	Renderer* renderer = Renderer::GetInstance();

	renderer->ClearScreen(Rgba::BLUE);
	renderer->SetCurrentCamera(renderer->GetUICamera());

	BitmapFont* font = AssetDB::CreateOrGetBitmapFont("Data/Images/Fonts/Default.png");
	AABB2 loadingBounds = AABB2(Vector2(0.35f * Window::GetInstance()->GetAspect() * Renderer::UI_ORTHO_HEIGHT, 0.3f * Renderer::UI_ORTHO_HEIGHT), Vector2(0.65f * Window::GetInstance()->GetAspect() * Renderer::UI_ORTHO_HEIGHT, 0.7f * Renderer::UI_ORTHO_HEIGHT));

	renderer->DrawTextInBox2D("Initializing Engine...", loadingBounds, Vector2(0.5f, 0.5f), 50.f, TEXT_DRAW_OVERRUN, font);
	renderer->EndFrame();
}


//-----------------------------------------------------------------------------------------------
// Checks for input this frame and makes state changes accordingly
//
void App::ProcessInput()
{
	PROFILE_LOG_SCOPE("App::ProcessInput");

	// Check to take a screenshot at the end of this frame
	if (InputSystem::GetInstance()->WasKeyJustPressed(InputSystem::KEYBOARD_F8))
	{
		Renderer::GetInstance()->SaveScreenshotAtEndOfFrame("Hello");
	}

	// Reload shader check
	if (InputSystem::GetInstance()->WasKeyJustPressed(InputSystem::KEYBOARD_F9))
	{
		AssetDB::ReloadShaderPrograms();
	}

	// DevConsole toggle check here in run frame because reasons
	if (InputSystem::GetInstance()->WasKeyJustPressed(InputSystem::KEYBOARD_TILDE))
	{
		DevConsole::ToggleConsole();
	}

	// Only process game input if the DevConsole is not open
	if (!DevConsole::IsDevConsoleOpen())
	{
		if (Profiler::IsProfilerOpen())
		{
			Profiler::GetInstance()->ProcessInput();
		}
		else
		{
			Game::GetInstance()->ProcessInput();
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Sets up the global blackboard for game settings and external information
//
void App::SetupGameConfigBlackboard()
{
	UNIMPLEMENTED();
	/*
	tinyxml2::XMLDocument configDocument;
	configDocument.LoadFile("Data/GameConfig.xml");
	g_gameConfigBlackboard = new Blackboard();
	tinyxml2::XMLElement* currElement = configDocument.RootElement();

	while (currElement != nullptr)
	{
		g_gameConfigBlackboard->PopulateFromXmlElementAttributes(*currElement);
		currElement = currElement->NextSiblingElement();
	}
	*/
}


//-----------------------------------------------------------------------------------------------
// Adds the App-specific commands to the command registry
//
void App::RegisterAppCommands() const
{
	Command::Register("quit", "Closes the application", Command_Quit);
	Command::Register("exit", "Closes the application", Command_Quit);
}


//-----------------------------------------------------------------------------------------------
// Accessor used by Main_Win32 to determine when the app was closed
//
bool App::IsQuitting() const
{
	return m_isQuitting;
}


//-----------------------------------------------------------------------------------------------
// Returns the time in milliseconds since the App first launched
//
float App::GetTimeSinceProgramStart() const
{
	return m_timeSinceProgramStart;
}


//-----------------------------------------------------------------------------------------------
// Returns the singleton App instance
//
App* App::GetInstance()
{
	return s_instance;
}


//-----------------------------------------------------------------------------------------------
// Deletes the app instance
//
void App::Shutdown()
{
	delete s_instance;
	s_instance = nullptr;
}


//-----------------------------------------------------------------------------------------------
// Sets the App's quit bool to true, so Main_Win32 will destroy it
//
void App::Quit()
{
	m_isQuitting = true;
}
