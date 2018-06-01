/************************************************************************/
/* Project: Protogame2D
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
		

	//-----Game Functions-----
	void RunFrame();					// Calls update and render on all entities

	//-----Boolean Mutators-----
	void Quit();						// Closes the App instance


	//-----Boolean Accessors-----
	bool IsQuitting() const;			// returns true if the app is quitting

	
private:
	//-----Private Methods-----

	void	Update();						// Updates the game each frame
	void	Render() const;				// Draws the objects to screen

	float	GetDeltaTime() const;			// Calculates the time from the last frame
	void	SetupGameConfigBlackboard();	// Sets up the global blackboard for game settings and external information
	void	LoadResources() const;			// Loads textures and audio from disk


private:

	//-----Private Data-----
	bool m_isQuitting = false;			// Boolean for tracking when to close the app
	bool m_fastTime = false;			// Whether delta time should be larger, for fast gameplay


	static const int MIN_FRAMERATE = 20;
};

extern App* g_theApp;
