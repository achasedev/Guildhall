/************************************************************************/
/* Project: Protogame2D
/* File: Main_Win32.cpp
/* Author: Andrew Chase
/* Date: March 24th, 2018
/* Bugs: None
/* Description: Creates a window with an OpenGL canvas to draw on
				Serves as the entry point for the program
/************************************************************************/
#define WIN32_LEAN_AND_MEAN		// Always #define this before #including <windows.h>
#include <windows.h>			// #include this (massive, platform-specific) header in very few places
#include "Game/Framework/App.hpp"
#include "Game/Framework/GameCommon.hpp"

#include "Engine/Core/Window.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Rendering/OpenGL/glFunctions.hpp"

#pragma comment(lib, "ThirdParty/assimp/lib/assimp-vc140-mt.lib" )
#define ASSIMP_DLL

#define UNUSED(x) (void)(x);

//-----------------------------------------------------------------------------------------------
// Windows message handler that is used to update the InputSystem on key state
//
bool AppMessageHandler(unsigned int msg, size_t wparam, size_t lparam)
{
	UNUSED(lparam);
	UNUSED(wparam);

	switch( msg )
	{
		// App close requested via "X" button, or right-click "Close Window" on task bar, or "Close" from system menu, or Alt-F4
	case WM_CLOSE:		
	{
		App::GetInstance()->Quit();
		return false; // "Consumes" this message (tells Windows "okay, we handled it")
	}
	} 

	return true;
}


//-----------------------------------------------------------------------------------------------
// Sets up game framework (creating a window, gl context, and App for managing Engine tools)
//
void Initialize()
{ 
	// Creates a Windows window for the game
	Window* theWindow = Window::Initialize(1.777777f, "DFS - Animation : Andrew Chase");	// 1.77777 = 16:9 aspect ratio
	theWindow->RegisterHandler(AppMessageHandler);

	// Set up the modern context
	GLStartup();

	// Create the App and all Engine systems to be used
	App::Initialize();	
}


//-----------------------------------------------------------------------------------------------
// Shuts down framework before exiting
//
void Shutdown()
{
	// Destroy the global App instance
	App::Shutdown();

	// Clean up the GL context
	GLShutdown();

	// Shutdown the window used for the context
	Window::Shutdown();
}


//-----------------------------------------------------------------------------------------------
// Master MAIN function for the program, loops until the App is closed
//
int WINAPI WinMain( HINSTANCE applicationInstanceHandle, HINSTANCE, LPSTR commandLineString, int )
{
	UNUSED( commandLineString );
	UNUSED( applicationInstanceHandle );

	Initialize();
  
	// Program main loop; keep running frames until it's time to quit
	while(!App::GetInstance()->IsQuitting())
	{
		App::GetInstance()->RunFrame();
	}

	Shutdown();
	return 0;
}
