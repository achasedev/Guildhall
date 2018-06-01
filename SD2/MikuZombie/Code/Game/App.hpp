/************************************************************************/
/* File: App.hpp
/* Author: Andrew Chase
/* Date: March 24th, 2018
/* Description: Class used to manage a Game instance
/************************************************************************/
#pragma once
#include "Game/Game.hpp"


class App 
{
public:	
	//-----Game Functions-----
	void RunFrame();		// Calls update and render on all entities

	//-----Mutators-----
	void Quit();			

	//-----Accessors-----
	bool	IsQuitting() const;
	float	GetTimeSinceProgramStart() const;

	//-----Singleton class functions-----
	static void Initialize();
	static void Shutdown();
	static App* GetInstance();

private:
	//-----Private Methods-----

	App();		
	~App();	
	App(const App& copy) = delete;

private:
	//-----Private Data-----

	bool	m_isQuitting = false;			// Boolean for tracking when to close the app
	bool	m_engineSystemsInitialized = false;	// Flag to denote when the engine systems are created
	float	m_timeSinceProgramStart;		// Time since the app first launched, in milliseconds

	void	Update();						// Updates the game each frame
	void	Render() const;					// Draws the objects to screen
		void RenderInitScreen() const;	// Draws an initialization screen while the engine systems are being created
	void	ProcessInput();					// Process input received this frame

	void	SetupGameConfigBlackboard();	// Sets up the global blackboard for game settings and external information
	void	RegisterAppCommands() const;	// Registers all commands and command callbacks to the command registry

	static App* s_instance;					// The singleton App instance
};
