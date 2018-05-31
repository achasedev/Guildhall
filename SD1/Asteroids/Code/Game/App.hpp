/************************************************************************/
/* Project: SD1 C27 A2 - Asteroids
/* File: App.hpp
/* Author: Andrew Chase
/* Date: September 3rd, 2017
/* Bugs: None
/* Description: Class used to manage a Game instance of Asteroids
/************************************************************************/
#pragma once
#include "Game/Game.hpp"


class App 
{
public:
	//-----Constructors-----
	App();		
	~App();							
		

	//-----Game Functions-----
	void RunFrame();					// Calls update and render on all entities

	//-----Boolean Mutators-----
	void TogglePaused();				// Toggles the App as paused/unpaused
	void Quit();						// Closes the App instance


	//-----Boolean Accessors-----
	bool IsPaused() const;											// returns true if the game is currently paused
	bool IsQuitting() const;										// returns true if the app is quitting

private:

	//-----Private Data-----
	Game *m_theGame;					// The game instanced being managed
	bool m_timeSlowed = false;			// Boolean for time slowing mechanic
	bool m_isPaused = false;			// Boolean members used for App pausing
	bool m_isQuitting = false;			// Boolean for tracking when to close the app

	void Update();						// Updates the game each frame
	void Render() const;				// Draws the objects to screen
};

extern App* g_theApp;
