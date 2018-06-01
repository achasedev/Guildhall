/************************************************************************/
/* Project: Protogame2D
/* File: Main_Win32.cpp
/* Author: Andrew Chase
/* Date: January 25th, 2017
/* Bugs: None
/* Description: Creates a window with an OpenGL canvas to draw on
				Serves as the entry point for the program
/************************************************************************/
#define WIN32_LEAN_AND_MEAN		// Always #define this before #including <windows.h>
#include <windows.h>			// #include this (massive, platform-specific) header in very few places
#include "Game/App.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/Window.hpp"
#include "Engine/Renderer/glFunctions.hpp"

#define UNUSED(x) (void)(x);


//-----------------------------------------------------------------------------------------------
// Windows message handler that is used to update the InputSystem on key state
//
bool AppMessageHandler(unsigned int msg, size_t wparam, size_t lparam)
{
	UNUSED(lparam);

	// Process the message and pass to the input system if a key was pressed
	unsigned char keyCode = (unsigned char) wparam;
	switch( msg )
	{
		// App close requested via "X" button, or right-click "Close Window" on task bar, or "Close" from system menu, or Alt-F4
	case WM_CLOSE:		
	{
		App::GetInstance()->Quit();
		return false; // "Consumes" this message (tells Windows "okay, we handled it")
	}

	// Raw physical keyboard "key-was-just-pressed" event (case-insensitive, not translated)
	case WM_KEYDOWN:
	{
		// Process which key was pressed
		InputSystem::GetInstance()->OnKeyPressed(keyCode);
		break;
	}

	// Raw physical keyboard "key-was-just-released" event (case-insensitive, not translated)
	case WM_KEYUP:
	{
		// Process which key was released
		InputSystem::GetInstance()->OnKeyReleased(keyCode);
		break;
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
	Window* theWindow = Window::Initialize(1.777777f, "Tactics - Andrew Chase");	// 1.77777 = 16:9 aspect ratio
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
