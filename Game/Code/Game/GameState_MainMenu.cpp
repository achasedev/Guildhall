/************************************************************************/
/* File: GameState_MainMenu.cpp
/* Author: Andrew Chase
/* Date: March 24th, 2018
/* Description: Implementation of the GameState_MainMenu class
/************************************************************************/
#include "Game/App.hpp"
#include "Game/GameCommon.hpp"
#include "Game/GameState_Ready.hpp"
#include "Game/GameState_Playing.hpp"
#include "Game/GameState_MainMenu.hpp"

#include "Engine/Core/Window.hpp"
#include "Engine/Core/AssetDB.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Input/InputSystem.hpp"

//-----------------------------------------------------------------------------------------------
// Default constructor
//
GameState_MainMenu::GameState_MainMenu()
	: m_cursorPosition(0)
{
	float aspect = Window::GetInstance()->GetAspect();
	float height = Renderer::UI_ORTHO_HEIGHT;

	m_menuBounds = AABB2(Vector2(0.1f * aspect * height, 0.1f * height), Vector2(0.9f * aspect * height, 0.4f * height));
	m_fontHeight = 100.f;

	m_menuOptions.push_back("Press Space to proceed to ready state.");
}


//-----------------------------------------------------------------------------------------------
// Checks for input related to this GameState and changes state accordingly
//
void GameState_MainMenu::ProcessInput()
{
	InputSystem* input = InputSystem::GetInstance();

	// Moving down
	bool keyPressedDown = input->WasKeyJustPressed(InputSystem::KEYBOARD_DOWN_ARROW);
	if (keyPressedDown)
	{
		m_cursorPosition++;
		if (m_cursorPosition > (int) (m_menuOptions.size()) - 1)
		{
			m_cursorPosition = 0;
		}
	}

	// Moving up
	bool keyPressedUp = input->WasKeyJustPressed(InputSystem::KEYBOARD_UP_ARROW);
	if (keyPressedUp)
	{
		m_cursorPosition--;
		if (m_cursorPosition < 0)
		{
			m_cursorPosition = (int) (m_menuOptions.size()) - 1;
		}
	}

	// Selection
	if (input->WasKeyJustPressed(InputSystem::KEYBOARD_SPACEBAR))
	{
		Game::TransitionToGameState(new GameState_Ready());
	}

	// Quit
	if (input->WasKeyJustPressed(InputSystem::KEYBOARD_ESCAPE))
	{
		App::GetInstance()->Quit();
	}
}


//-----------------------------------------------------------------------------------------------
// Updates the MainMenu state
//
void GameState_MainMenu::Update()
{
}


//-----------------------------------------------------------------------------------------------
// Renders the Main Menu
//
void GameState_MainMenu::Render() const
{
	Renderer* renderer = Renderer::GetInstance();
	renderer->SetCurrentCamera(renderer->GetUICamera());

	// Setup the renderer
	renderer->ClearScreen(Rgba::LIGHT_BLUE);
	renderer->Draw2DQuad(m_menuBounds, AABB2::UNIT_SQUARE_OFFCENTER, Rgba::BLUE, AssetDB::GetSharedMaterial("UI"));

	// Draw the menu options
	BitmapFont* font = AssetDB::CreateOrGetBitmapFont("Default.png");
	AABB2 currentTextBounds = m_menuBounds;

	for (int menuIndex = 0; menuIndex < static_cast<int>(m_menuOptions.size()); ++menuIndex)
	{
		Rgba color = Rgba::WHITE;
		if (menuIndex == m_cursorPosition)
		{
			color = Rgba::YELLOW;
		}
		renderer->DrawTextInBox2D(m_menuOptions[menuIndex].c_str(), currentTextBounds, Vector2(0.5f, 0.5f), m_fontHeight, TEXT_DRAW_SHRINK_TO_FIT, font, color);
		currentTextBounds.Translate(Vector2(0.f, -m_fontHeight));
	}
}


//-----------------------------------------------------------------------------------------------
// Called when the game transitions into this state, before the first update
//
void GameState_MainMenu::Enter()
{
}


//-----------------------------------------------------------------------------------------------
// Called when the game transitios out of this state, before deletion
//
void GameState_MainMenu::Leave()
{
}


//-----------------------------------------------------------------------------------------------
// Processes the enter command on a menu selection
//
void GameState_MainMenu::ProcessMenuSelection() const
{
}
