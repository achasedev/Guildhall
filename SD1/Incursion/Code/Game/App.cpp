/************************************************************************/
/* Project: Incursion
/* File: App.cpp
/* Author: Andrew Chase
/* Date: September 23rd, 2017
/* Bugs: None
/* Description: Handles communication between the engine and the game
/************************************************************************/
#include "Game/App.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Math/MathUtils.hpp"

//-----------------------------------------------------------------------------------------------
// Default constructor - creates the Game and initializes it to the initial game state
//
App::App()
	: m_isQuitting(false)
	, m_isRunningHighSpeed(false)
{
	g_theRenderer = new Renderer();

	g_theInputSystem = new InputSystem();

	g_theAudioSystem = new AudioSystem();
}


//-----------------------------------------------------------------------------------------------
// Destructor that deletes the Game instance
//
App::~App()
{
	delete g_theGame;
	g_theGame = nullptr;

	delete g_pickUpSpriteSheet;
	g_pickUpSpriteSheet = nullptr;

	delete g_tileSpriteSheet;
	g_tileSpriteSheet = nullptr;

	delete g_explosionSpriteSheet;
	g_explosionSpriteSheet = nullptr;

	delete g_theAudioSystem;
	g_theAudioSystem = nullptr;

	delete g_theInputSystem;
	g_theInputSystem = nullptr;

	delete g_theRenderer;
	g_theRenderer = nullptr;
}


//-----------------------------------------------------------------------------------------------
// Loads the minimal resources needed to draw the loading screen
//
void App::LoadInitialResources() const
{
	g_theRenderer->CreateOrGetTexture(PLAYER_BASE_TEXTURE_PATH);
	g_theRenderer->CreateOrGetTexture(PLAYER_TURRET_TEXTURE_PATH);
	g_theRenderer->CreateOrGetTexture(LOADING_SCREEN_TEXTURE_PATH);
	
	// Load a sound too!

}


//-----------------------------------------------------------------------------------------------
// Loads all needed textures for the game, performed at start up for efficiency
//
void App::LoadGameTextures() const
{
	g_theRenderer->CreateOrGetTexture(ENEMY_TANK_LIGHT_TEXTURE_PATH);
	g_theRenderer->CreateOrGetTexture(ENEMY_TANK_MEDIUM_TEXTURE_PATH);
	g_theRenderer->CreateOrGetTexture(ENEMY_TANK_HEAVY_TEXTURE_PATH);
	g_theRenderer->CreateOrGetTexture(ENEMY_TANK_TD_TEXTURE_PATH);
	g_theRenderer->CreateOrGetTexture(FRIENDLY_TANK_LIGHT_TEXTURE_PATH);
	g_theRenderer->CreateOrGetTexture(FRIENDLY_TANK_MEDIUM_TEXTURE_PATH);
	g_theRenderer->CreateOrGetTexture(FRIENDLY_TANK_HEAVY_TEXTURE_PATH);
	g_theRenderer->CreateOrGetTexture(FRIENDLY_TANK_TD_TEXTURE_PATH);
	g_theRenderer->CreateOrGetTexture(TURRET_BASE_TEXTURE_PATH);
	g_theRenderer->CreateOrGetTexture(TURRET_TOP_TEXTURE_PATH);
	g_theRenderer->CreateOrGetTexture(BULLET_TEXTURE_PATH);

	g_theRenderer->CreateOrGetTexture(PAUSE_SCREEN_TEXTURE_PATH);
	g_theRenderer->CreateOrGetTexture(GAME_OVER_SCREEN_TEXTURE_PATH);
	g_theRenderer->CreateOrGetTexture(TITLE_SCREEN_TEXTURE_PATH);
	g_theRenderer->CreateOrGetTexture(TITLE_SCREEN_FLASH_TEXTURE_PATH);
	g_theRenderer->CreateOrGetTexture(VICTORY_SCREEN_TEXTURE_PATH);
	g_theRenderer->CreateOrGetTexture(VICTORY_SCREEN_FLASH_TEXTURE_PATH);
	g_theRenderer->CreateOrGetTexture(VICTORY_SCREEN_FADE_TEXTURE_PATH);

	// Make the SpiteSheets
	Texture* tileSetTexture = g_theRenderer->CreateOrGetTexture(TERRAIN_TEXTURE_PATH);
	g_tileSpriteSheet = new SpriteSheet(*tileSetTexture, IntVector2(8, 8));

	Texture* explosionTexture = g_theRenderer->CreateOrGetTexture(EXPLOSION_TEXTURE_PATH);
	g_explosionSpriteSheet = new SpriteSheet(*explosionTexture, IntVector2(5, 5));

	Texture* pickUpTexture = g_theRenderer->CreateOrGetTexture(PICKUP_TEXTURE_PATH);
	g_pickUpSpriteSheet = new SpriteSheet(*pickUpTexture, IntVector2(4, 4));
}


//-----------------------------------------------------------------------------------------------
// Loads the audio assets for the game into memory 
//
void App::LoadGameAudio() const
{
	g_theAudioSystem->CreateOrGetSound(LEVEL_MUSIC_PATH);
	g_theAudioSystem->CreateOrGetSound(MAIN_MENU_MUSIC_PATH);
	g_theAudioSystem->CreateOrGetSound(VICTORY_MUSIC_PATH);

	g_theAudioSystem->CreateOrGetSound(PLAYER_TANK_ENGINE_SOUND_PATH);
	g_theAudioSystem->CreateOrGetSound(SHOT_FIRED_SOUND_PATH);
	g_theAudioSystem->CreateOrGetSound(PLAYER_DAMAGE_TAKEN_SOUND_PATH);
	g_theAudioSystem->CreateOrGetSound(ENEMY_DAMAGE_TAKEN_SOUND_PATH);

	g_theAudioSystem->CreateOrGetSound(PLAYER_PAUSE_SOUND_PATH);
	g_theAudioSystem->CreateOrGetSound(PLAYER_UNPAUSE_SOUND_PATH);
	g_theAudioSystem->CreateOrGetSound(PLAYER_RESPAWN_SOUND_PATH);
	g_theAudioSystem->CreateOrGetSound(MAP_CHANGE_SOUND_PATH);

	g_theAudioSystem->CreateOrGetSound(EXPLOSION1_SOUND_PATH);
	g_theAudioSystem->CreateOrGetSound(EXPLOSION2_SOUND_PATH);
	g_theAudioSystem->CreateOrGetSound(EXPLOSION3_SOUND_PATH);
	g_theAudioSystem->CreateOrGetSound(EXPLOSION4_SOUND_PATH);
	g_theAudioSystem->CreateOrGetSound(EXPLOSION5_SOUND_PATH);

	g_theAudioSystem->CreateOrGetSound(INVINCIBILITY_SOUND_PATH);
	g_theAudioSystem->CreateOrGetSound(HEALTH_SOUND_PATH);
	g_theAudioSystem->CreateOrGetSound(REINFORCEMENTS_SOUND_PATH);
	g_theAudioSystem->CreateOrGetSound(SPEEDBOOST_SOUND_PATH);
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
	if (m_appInitialized)
	{
		float deltaTime = ProcessInput();
		g_theGame->Update(deltaTime);
	}
	else if (m_loadingScreenDrawn)
	{
		LoadGameResources();
	}
	else
	{
		LoadInitialResources();
	}
}


//-----------------------------------------------------------------------------------------------
// Checks for input that changes the state of the App
//
float App::ProcessInput()
{
	// Get deltaTime
	float deltaTime = GetDeltaTime();

	if (g_theInputSystem->IsKeyPressed('T'))
	{
		deltaTime *= 0.1f;
	}

	if (g_theInputSystem->WasKeyJustPressed(InputSystem::KEYBOARD_F6))
	{
		m_isRunningHighSpeed = !m_isRunningHighSpeed;
	}

	if (m_isRunningHighSpeed)
	{
		deltaTime *= 5.0f;
	}

	if (g_theGame->IsPaused() || g_theGame->IsGameOver())
	{
		deltaTime = 0.f;
	}

	// Check for quitting
	if (g_theGame->IsAtMainMenu())
	{
		if (g_theInputSystem->WasKeyJustPressed(InputSystem::KEYBOARD_ESCAPE) || g_theInputSystem->GetController(0).WasButtonJustPressed(XBOX_BUTTON_BACK))
		{
			Quit();
		}
	}

	return deltaTime;
}

//-----------------------------------------------------------------------------------------------
// Tells the Game object to render all Entities
//
void App::Render()
{
	if (m_appInitialized)
	{
		g_theRenderer->ClearScreen(Rgba::BLACK);
		g_theGame->Render();
	}
	else if (!m_loadingScreenDrawn)
	{
		DrawLoadingScreen();
	}
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

	return deltaTime;
}


//-----------------------------------------------------------------------------------------------
// Draws a loading screen and plays a sound, then begins to load all assets necessary for the game
//
void App::LoadGameResources()
{
	// Draw the screen
	DrawLoadingScreen();

	// Load Textures
	LoadGameTextures();
	LoadGameAudio();

	g_theGame = new Game();

	m_appInitialized = true;
}


//-----------------------------------------------------------------------------------------------
// Draws the loading screen texture to the entire screen
//
void App::DrawLoadingScreen()
{
	// Draw the loading screen texture over the entire screen
	AABB2 screenBounds = AABB2(Vector2(0.f, 0.f), Vector2(16.f, 9.f));
	g_theRenderer->SetOrtho(screenBounds);

	Texture* loadingScreenTexture = g_theRenderer->CreateOrGetTexture(LOADING_SCREEN_TEXTURE_PATH);
	Texture* tankBaseTexture = g_theRenderer->CreateOrGetTexture(PLAYER_BASE_TEXTURE_PATH);
	Texture* tankTurretTexture = g_theRenderer->CreateOrGetTexture(PLAYER_TURRET_TEXTURE_PATH);

	// Make the background light blue
	g_theRenderer->ClearScreen(Rgba::LIGHT_BLUE);

	// Draw a bunch of player tanks in random locations
	for (int i = 0; i < 500; i++)
	{
		Vector2 randomBottomLeft = Vector2(GetRandomFloatInRange(-1.f, 17.f), GetRandomFloatInRange(-1.f, 10.f));
		Vector2 randomTopRight = randomBottomLeft + (GetRandomFloatInRange(2.f, 5.f) * Vector2(1.f, 1.f));

		AABB2 tankBounds = AABB2(randomBottomLeft, randomTopRight);
		g_theRenderer->DrawTexturedAABB2(tankBounds, *tankBaseTexture, Vector2(0.f, 1.f), Vector2(1.f, 0.f), Rgba::WHITE);
		g_theRenderer->DrawTexturedAABB2(tankBounds, *tankTurretTexture, Vector2(0.f, 1.f), Vector2(1.f, 0.f), Rgba::WHITE);
	}
	
	// Draw the "Now loading" font overtop
	g_theRenderer->DrawTexturedAABB2(screenBounds, *loadingScreenTexture, Vector2(0.f, 1.f), Vector2(1.f, 0.f), Rgba::WHITE);

	m_loadingScreenDrawn = true;
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
