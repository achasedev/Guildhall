/************************************************************************/
/* Project: Incursion
/* File: App.hpp
/* Author: Andrew Chase
/* Date: September 23rd, 2017
/* Bugs: None
/* Description: Class used to manage a Game instance
/************************************************************************/
#pragma once
#include "Game/Game.hpp"


class App 
{
public:
	//-----Constructors-----
	App();		
	~App();							
		

	void LoadInitialResources() const;	// Loads the resources needed to draw the loading screen

	//-----Game Functions-----
	void LoadGameTextures() const;		// Loads all textures into the GPU for the game
	void LoadGameAudio() const;			// Loads all audio assets into memory
	void RunFrame();					// Calls update and render on all entities

	//-----Boolean Mutators-----
	void Quit();						// Closes the App instance


	//-----Boolean Accessors-----
	bool IsQuitting() const;			// returns true if the app is quitting

private:

	//-----Private Data-----
	bool m_isQuitting = false;			// Boolean for tracking when to close the app
	bool m_isRunningHighSpeed;			// Run frames at larger deltaTimes

	bool m_loadingScreenDrawn = false;
	bool m_appInitialized = false;		// True when the resources are loaded for the game

	void Update();						// Updates the game each frame
	float ProcessInput();				// Checks for input that changes the state of the App, returns deltaTime
	void Render();						// Draws the objects to screen
	
	float GetDeltaTime() const;			// Calculates the time from the last frame
	void LoadGameResources();
	void DrawLoadingScreen();

	static const int MIN_FRAMERATE = 20;
};

extern App* g_theApp;
