/************************************************************************/
/* Project: Incursion
/* File: Game.hpp
/* Author: Andrew Chase
/* Date: September 23rd, 2017
/* Bugs: None
/* Description: Class used for managing and updating game objects and 
/*              mechanics
/************************************************************************/
#pragma once
#include "Engine/Math/Vector2.hpp"
#include "Game/World.hpp"
#include "Game/PlayerTank.hpp"
#include "Engine/Audio/AudioSystem.hpp"

enum GameState
{
	GAME_STATE_ERROR = -1,
	GAME_STATE_MAINMENU,
	GAME_STATE_PLAYING,
	GAME_STATE_PAUSED,
	GAME_STATE_DYING,
	GAME_STATE_GAMEOVER, 
	GAME_STATE_VICTORY,
	NUM_GAME_STATES
};

class Game
{
	
public:

	Game();
	~Game();

	//-----Public Methods-----
	void Update(float deltaTime);			// Updates all game object states, called each frame
	void Render() const;					// Renders all game objects to screen, called each frame

	bool IsPaused() const;				// Returns true if the game is paused
	bool IsGameOver() const;			// Returns true if the game is over (player died)
	bool IsAtMainMenu() const;			// Returns true if the game is at the main menu

	void PauseGame();
	void UnpauseGame();

	bool IsDrawingDebugInfo() const;

private:
			
	//-----Private Methods-----
	void StartGame();
	void GoToMainMenu();

	void ProcessInputThisFrame();			// Parses input this frame for updating game state								

	void DrawPauseScreen() const;			// Draws the pause screen texture over the screen
	void DrawGameOverScreen() const;		// Draws the game over texture over the entire screen

	void DrawMainMenuScreen() const;		// Draws the main menu screen (in attract mode)
	void DrawMainMenuFlashText() const;

	void DrawVictoryScreen() const;
	void DrawVictoryFlashText() const;
	void DrawVictoryFadeText() const;

	void FadeOutScreenTextures();

	void PlayMusic(std::string pathOfSongToPlay, float volume);		// Sets the given SoundID for a song as the current song, and begins playing it

private:

	//-----Private Data-----
	World* m_theWorld;										// Collection of maps to play

	//bool m_gameOver = false;								// Flag for when the game is over
	float m_timeUntilDeathScreen = GAME_OVER_DELAY;			// Timer to delay showing the game over screen after the player dies
	//bool m_isPaused = false;								// Flag for when the game is paused
	bool m_inDebugMode = false;								// Indicates whether debug information is rendered or not
	//bool m_atMainMenu = true;								// Indicates when the game hasn't started and is in "attract" mode
	GameState m_currentState;
	SoundPlaybackID m_currentSongPlaying;					// To fade the pause screen in and out

	float m_pauseScreenAlphaScalar;
	float m_gameOverAlphaScalar;

	static const float GAME_OVER_DELAY;						// Delay after a player dies should the game over texture be drawn
	static const float FADE_SPEED;
};
