/************************************************************************/
/* Project: Incursion
/* File: Game.cpp
/* Author: Andrew Chase
/* Date: September 23rd, 2017
/* Bugs: None
/* Description: Implementation of the Game class, used for managing and 
/*				updating game objects and mechanics
/************************************************************************/
#include "Game.hpp"
#include "Game/GameCommon.hpp"
#include "Game/TileDefinition.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Math/Disc2.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Game/Camera.hpp"


const float Game::GAME_OVER_DELAY = 2.f;
const float Game::FADE_SPEED	  = 0.03f;
//-----------------------------------------------------------------------------------------------
// Default constructor, initialize any game members here
//
Game::Game()
	:m_currentState(GAME_STATE_MAINMENU)
	, m_pauseScreenAlphaScalar(0.f)
	, m_gameOverAlphaScalar(0.f)
{
	// Todo: figure out where to move this
	TileDefinition::InitializeTileDefinitions();
	PlayMusic(MAIN_MENU_MUSIC_PATH, 1.0f);
}


//-----------------------------------------------------------------------------------------------
// Destructor - clean up any allocated members
//
Game::~Game()
{
	// Delete the sprite sheets
	delete g_explosionSpriteSheet;
	g_explosionSpriteSheet = nullptr;

	delete g_tileSpriteSheet;
	g_tileSpriteSheet = nullptr;

	delete g_thePlayer;
	g_thePlayer = nullptr;

	delete g_theCamera;
	g_theCamera = nullptr;

	delete m_theWorld;
	m_theWorld = nullptr;
}


//-----------------------------------------------------------------------------------------------
// Update the movement variables of every entity in the world, as well as update the game state
// based on the input this frame
//
void Game::Update(float deltaTime)
{
	// Process game-related input
	ProcessInputThisFrame();

	switch (m_currentState)
	{
	case GAME_STATE_MAINMENU:
		break;
	case GAME_STATE_PLAYING:
		m_theWorld->Update(deltaTime);
		FadeOutScreenTextures();

		if (g_thePlayer->IsMarkedForDeath())
		{
			g_thePlayer->PerformDeathAnimation();
			m_currentState = GAME_STATE_DYING;
		}
		else
		{
			g_theCamera->Update(deltaTime);
		}

		// Check for game completion
		if (m_theWorld->AreAllLevelsComplete())
		{
			m_currentState = GAME_STATE_VICTORY;
			PlayMusic(VICTORY_MUSIC_PATH, 0.8f);
		}
		break;
	case GAME_STATE_PAUSED:
		m_theWorld->Update(deltaTime);
		// Update the pause scalar
		m_pauseScreenAlphaScalar += FADE_SPEED;
		m_pauseScreenAlphaScalar = ClampFloatZeroToOne(m_pauseScreenAlphaScalar);
		break;
	case GAME_STATE_DYING:
		m_theWorld->Update(deltaTime);
		m_timeUntilDeathScreen -= deltaTime;	
		if (m_timeUntilDeathScreen <= 0.f)
		{
			m_currentState = GAME_STATE_GAMEOVER;
			// Turn down the music!
			g_theAudioSystem->SetSoundPlaybackVolume(m_currentSongPlaying, 0.2f);
		}
		break;
	case GAME_STATE_GAMEOVER:
		m_theWorld->Update(deltaTime);

		m_gameOverAlphaScalar += FADE_SPEED;
		m_gameOverAlphaScalar = ClampFloatZeroToOne(m_gameOverAlphaScalar);
		break;
	default:
		break;
	}
}


//-----------------------------------------------------------------------------------------------
// Draws all entities to the screen
//
void Game::Render() const
{

	if (m_currentState == GAME_STATE_MAINMENU)
	{
		DrawMainMenuScreen();
	}
	else if (m_currentState == GAME_STATE_VICTORY)
	{
		DrawVictoryScreen();
	}
	else
	{
		m_theWorld->Render();

		DrawPauseScreen();
		DrawGameOverScreen();
	}
}


//-----------------------------------------------------------------------------------------------
// Returns whether or not the game is paused
//
bool Game::IsPaused() const
{
	return (m_currentState == GAME_STATE_PAUSED);
}


//-----------------------------------------------------------------------------------------------
// Returns whether or not the game is over (player dead)
//
bool Game::IsGameOver() const
{
	return (m_currentState == GAME_STATE_GAMEOVER);
}


//-----------------------------------------------------------------------------------------------
// Returns whether or not the game is at the main menu
//
bool Game::IsAtMainMenu() const
{
	return (m_currentState == GAME_STATE_MAINMENU);
}


//-----------------------------------------------------------------------------------------------
// Pauses the game
//
void Game::PauseGame()
{
	m_currentState = GAME_STATE_PAUSED;

	// Turn down the music!
	g_theAudioSystem->SetSoundPlaybackVolume(m_currentSongPlaying, 0.2f);

	// Play pause sound effect
	SoundID pauseSound = g_theAudioSystem->CreateOrGetSound(PLAYER_PAUSE_SOUND_PATH);
	g_theAudioSystem->PlaySound(pauseSound);
}


//-----------------------------------------------------------------------------------------------
// Unpauses the game
//
void Game::UnpauseGame()
{
	m_currentState = GAME_STATE_PLAYING;

	// Turn up the music!
	g_theAudioSystem->SetSoundPlaybackVolume(m_currentSongPlaying, 0.4f);

	// Play pause sound effect
	SoundID unpauseSound = g_theAudioSystem->CreateOrGetSound(PLAYER_UNPAUSE_SOUND_PATH);
	g_theAudioSystem->PlaySound(unpauseSound);
	
}


//-----------------------------------------------------------------------------------------------
// Accessor to tell if the game is drawing the entity debug Information
//
bool Game::IsDrawingDebugInfo() const
{
	return m_inDebugMode;
}

//-----------------------------------------------------------------------------------------------
// Initializes the game's responsible data members and leaves the main menu screen
//
void Game::StartGame()
{
	m_theWorld = new World();
	g_theCamera = new Camera();
	g_thePlayer = new PlayerTank();
	m_theWorld->AddEntityToCurrentMap(g_thePlayer);

	m_theWorld->InitializeMapsAndEntities();

	m_currentState = GAME_STATE_PLAYING;
	
	PlayMusic(LEVEL_MUSIC_PATH, 0.4f);
}


//-----------------------------------------------------------------------------------------------
// Deletes the game's data members that it is responsible for and returns to the main menu
//
void Game::GoToMainMenu()
{
	delete g_thePlayer;


	delete g_theCamera;
	g_theCamera = nullptr;

	delete m_theWorld;
	m_theWorld = nullptr;
	g_thePlayer = nullptr;

	// Stop playing the level music
	PlayMusic(MAIN_MENU_MUSIC_PATH, 1.0f);
	m_currentState = GAME_STATE_MAINMENU;
}


//-----------------------------------------------------------------------------------------------
// Checks for game-related input this frame
//
void Game::ProcessInputThisFrame()
{
	if (g_theInputSystem->WasKeyJustPressed(InputSystem::KEYBOARD_F1))
	{
		m_inDebugMode = !m_inDebugMode;
	}

	// Get the various input separately
	bool pPressed = g_theInputSystem->WasKeyJustPressed('P');
	bool escPressed = g_theInputSystem->WasKeyJustPressed(InputSystem::KEYBOARD_ESCAPE);
	bool spacePressed = g_theInputSystem->WasKeyJustPressed(InputSystem::KEYBOARD_SPACEBAR);
	bool startPressed = g_theInputSystem->GetController(0).WasButtonJustPressed(XBOX_BUTTON_START);
	bool backPressed = g_theInputSystem->GetController(0).WasButtonJustPressed(XBOX_BUTTON_BACK); 

	switch (m_currentState)
	{
	case GAME_STATE_MAINMENU:
		if (spacePressed || startPressed)
		{
			StartGame();
		}
		break;

	case GAME_STATE_PAUSED:
		if (pPressed || startPressed)
		{
			UnpauseGame();
		}
		else if (escPressed|| backPressed)
		{
			GoToMainMenu();
		}
		break;

	case GAME_STATE_GAMEOVER:
		if (pPressed || startPressed)
		{
			// Reset the game over timer and respawn the player
			m_timeUntilDeathScreen = GAME_OVER_DELAY;
			g_thePlayer->Respawn();
			m_currentState = GAME_STATE_PLAYING;
			// Turn the music back up!
			g_theAudioSystem->SetSoundPlaybackVolume(m_currentSongPlaying, 0.5f);
		}
		break;

	case GAME_STATE_PLAYING:
		if (pPressed || escPressed || startPressed || backPressed)
		{
			PauseGame();
		}

		if (g_theInputSystem->WasKeyJustPressed('R'))
		{
			m_theWorld->StartFadeAndChangeLevel();
		}
		break;
	case GAME_STATE_VICTORY:
		if (pPressed || startPressed || escPressed || backPressed)
		{
			GoToMainMenu();
		}
	default:
		break;
	}
}


//-----------------------------------------------------------------------------------------------
// Draws the pause texture over the screen
//
void Game::DrawPauseScreen() const
{
	// Draw the texture over the entire screen
	AABB2 screenBounds = AABB2(Vector2(0.f, 0.f), Vector2(16.f, 9.f));
	g_theRenderer->SetOrtho(screenBounds);
	Texture* gameOverTexture = g_theRenderer->CreateOrGetTexture(PAUSE_SCREEN_TEXTURE_PATH);

	// Get the correct alpha
	Rgba drawColor = Rgba::WHITE;
	drawColor.ScaleAlpha(m_pauseScreenAlphaScalar);

	g_theRenderer->DrawTexturedAABB2(screenBounds, *gameOverTexture, Vector2(0.f, 1.f), Vector2(1.f, 0.f), drawColor);
}


//-----------------------------------------------------------------------------------------------
// Draws the game over texture if 3 seconds had expired since player death
//
void Game::DrawGameOverScreen() const
{
	// Draw the texture over the entire screen
	AABB2 screenBounds = AABB2(Vector2(0.f, 0.f), Vector2(16.f, 9.f));
	g_theRenderer->SetOrtho(screenBounds);

	// Get the correct alpha
	Rgba drawColor = Rgba::WHITE;
	drawColor.ScaleAlpha(m_gameOverAlphaScalar);

	Texture* pauseTexture = g_theRenderer->CreateOrGetTexture(GAME_OVER_SCREEN_TEXTURE_PATH);

	g_theRenderer->DrawTexturedAABB2(screenBounds, *pauseTexture, Vector2(0.f, 1.f), Vector2(1.f, 0.f), drawColor);
}


//-----------------------------------------------------------------------------------------------
// Draws the main menu over the entire screen
//
void Game::DrawMainMenuScreen() const
{
	// Draw the texture over the entire screen
	AABB2 screenBounds = AABB2(Vector2(0.f, 0.f), Vector2(16.f, 9.f));
	g_theRenderer->SetOrtho(screenBounds);
	Texture* titleTexture = g_theRenderer->CreateOrGetTexture(TITLE_SCREEN_TEXTURE_PATH);

	g_theRenderer->DrawTexturedAABB2(screenBounds, *titleTexture, Vector2(0.f, 1.f), Vector2(1.f, 0.f), Rgba::WHITE);

	DrawMainMenuFlashText();
}


//-----------------------------------------------------------------------------------------------
// Draws the flashing menu "Press start to begin" text
//
void Game::DrawMainMenuFlashText() const
{
	// Drawing parameters
	AABB2 screenBounds = AABB2(Vector2(0.f, 0.f), Vector2(16.f, 9.f));
	static float flashAlphaScalar = 0.f;
	static float fadeDir = FADE_SPEED;

	// Create the alpha color
	Rgba drawColor = Rgba::WHITE;
	drawColor.ScaleAlpha(flashAlphaScalar);

	// Draw the texture with the new color (alpha)
	Texture* titleFlashTexture = g_theRenderer->CreateOrGetTexture(TITLE_SCREEN_FLASH_TEXTURE_PATH);
	g_theRenderer->DrawTexturedAABB2(screenBounds, *titleFlashTexture, Vector2(0.f, 1.f), Vector2(1.f, 0.f), drawColor);

	// Update the flash scalar
	flashAlphaScalar += fadeDir;
	if (flashAlphaScalar >= 1.f || flashAlphaScalar <= 0.f)
	{
		flashAlphaScalar = ClampFloatZeroToOne(flashAlphaScalar);
		fadeDir *= -1.f;
	}
}

//-----------------------------------------------------------------------------------------------
// Draws the victory texture over the entire screen
//
void Game::DrawVictoryScreen() const
{
	// Draw the main texture over the entire screen
	AABB2 screenBounds = AABB2(Vector2(0.f, 0.f), Vector2(16.f, 9.f));
	g_theRenderer->SetOrtho(screenBounds);

	// Fade in the victory screen from black
	static float victoryAlphaScalar = 0.f;
	Rgba drawColor = Rgba::WHITE;
	drawColor.ScaleAlpha(victoryAlphaScalar);

	// Draw the texture to screen
	Texture* victoryTexture = g_theRenderer->CreateOrGetTexture(VICTORY_SCREEN_TEXTURE_PATH);
	g_theRenderer->DrawTexturedAABB2(screenBounds, *victoryTexture, Vector2(0.f, 1.f), Vector2(1.f, 0.f), drawColor);

	// Update the alpha scalar
	victoryAlphaScalar += FADE_SPEED;
	victoryAlphaScalar = ClampFloatZeroToOne(victoryAlphaScalar);

	// If the victory screen is faded in completely, then begin the other text fades
	if (victoryAlphaScalar == 1.0f)
	{
		// Wait 100 frames until drawing the fade in text
		static int framesUntilStartFadeIn = 50;
		if (framesUntilStartFadeIn <= 0)
		{
			DrawVictoryFadeText();
		}
		else
		{
			framesUntilStartFadeIn--; 
		}

		// Wait 200 frames until drawing the flashing text
		static int framesUntilStartFlashing = 150;
		if (framesUntilStartFlashing <= 0)
		{
			DrawVictoryFlashText();
		}
		else
		{
			framesUntilStartFlashing--; 
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Draws the flashing victory "Press start to return to the main menu" text
//
void Game::DrawVictoryFlashText() const
{
	// Drawing parameters
	AABB2 screenBounds = AABB2(Vector2(0.f, 0.f), Vector2(16.f, 9.f));
	static float flashAlphaScalar = 0.f;
	static float fadeMagnitude = FADE_SPEED;

	// Create the alpha color
	Rgba drawColor = Rgba::WHITE;
	drawColor.ScaleAlpha(flashAlphaScalar);

	// Draw the texture
	Texture* victoryFlashTexture = g_theRenderer->CreateOrGetTexture(VICTORY_SCREEN_FLASH_TEXTURE_PATH);
	g_theRenderer->DrawTexturedAABB2(screenBounds, *victoryFlashTexture, Vector2(0.f, 1.f), Vector2(1.f, 0.f), drawColor);

	// Update the fade alpha
	flashAlphaScalar += fadeMagnitude;
	if (flashAlphaScalar >= 1.f || flashAlphaScalar <= 0.f)
	{
		flashAlphaScalar = ClampFloatZeroToOne(flashAlphaScalar);
		fadeMagnitude *= -1.f;
	}
}


//-----------------------------------------------------------------------------------------------
// Draws the fade in text "...for now..." for the victory screen
//
void Game::DrawVictoryFadeText() const
{
	// Drawing parameters
	AABB2 screenBounds = AABB2(Vector2(0.f, 5.f), Vector2(16.f, 14.f));
	static float flashAlphaScalar = 0.f;
	static float fadeMagnitude = FADE_SPEED;

	// Create the alpha color
	Rgba drawColor = Rgba::WHITE;
	drawColor.ScaleAlpha(flashAlphaScalar);

	// Draw the texture
	Texture* victoryFadeTexture = g_theRenderer->CreateOrGetTexture(VICTORY_SCREEN_FADE_TEXTURE_PATH);
	g_theRenderer->DrawTexturedAABB2(screenBounds, *victoryFadeTexture, Vector2(0.f, 1.f), Vector2(1.f, 0.f), drawColor);

	// Update the fade alpha
	flashAlphaScalar += fadeMagnitude;
	flashAlphaScalar = ClampFloatZeroToOne(flashAlphaScalar);
}


//-----------------------------------------------------------------------------------------------
// Updates the scalars used to control alpha levels on the screen-wide texture draws (pause and
// game over)
//
void Game::FadeOutScreenTextures()
{
	// Update the pause scalar and the game over scalar
	m_pauseScreenAlphaScalar -= FADE_SPEED;
	m_pauseScreenAlphaScalar = ClampFloatZeroToOne(m_pauseScreenAlphaScalar);

	m_gameOverAlphaScalar -= FADE_SPEED;
	m_gameOverAlphaScalar = ClampFloatZeroToOne(m_gameOverAlphaScalar);
}


//-----------------------------------------------------------------------------------------------
// Sets the song given by pathOfSongToPlay as the current song
//
void Game::PlayMusic(std::string pathOfSongToPlay, float volume)
{
	g_theAudioSystem->StopSound(m_currentSongPlaying);
	SoundID songToPlay = g_theAudioSystem->CreateOrGetSound(pathOfSongToPlay);
	SoundPlaybackID songPlaying = g_theAudioSystem->PlaySound(songToPlay, true, volume);
	m_currentSongPlaying = songPlaying;
}
