/************************************************************************/
/* File: GameState_MainMenu.cpp
/* Author: Andrew Chase
/* Date: February 12th, 2018
/* Description: Implementation of the GameState_MainMenu class
/************************************************************************/
#include "Game/App.hpp"
#include "Game/GameCommon.hpp"
#include "Game/GameState_Playing.hpp"
#include "Game/GameState_MainMenu.hpp"
#include "Game/BoardStateDefinition.hpp"
#include "Engine/Core/Window.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Input/InputSystem.hpp"


//-----------------------------------------------------------------------------------------------
// Default constructor
//
GameState_MainMenu::GameState_MainMenu()
	: m_cursorPosition(0)
{
	m_menuBounds = AABB2(Vector2(0.35f * Window::GetInstance()->GetWindowAspect() * Renderer::UI_ORTHO_HEIGHT, 0.3f * Renderer::UI_ORTHO_HEIGHT), Vector2(0.65f * Window::GetInstance()->GetWindowAspect() * Renderer::UI_ORTHO_HEIGHT, 0.7f * Renderer::UI_ORTHO_HEIGHT));
	m_fontHeight = 75.f;

	CreateMenuFromLoadedDefinitions();
}


//-----------------------------------------------------------------------------------------------
// Checks for input related to this GameState and changes state accordingly
//
void GameState_MainMenu::ProcessInput()
{
	InputSystem* input = InputSystem::GetInstance();
	XboxController& controller = InputSystem::GetPlayerOneController();

	bool keyPressedDown = input->WasKeyJustPressed(InputSystem::KEYBOARD_DOWN_ARROW);
	bool stickPressedDown = (controller.WasStickJustPressed(XBOX_STICK_LEFT) && controller.GetCardinalStickDirection(XBOX_STICK_LEFT) == IntVector2::STEP_SOUTH);

	if (keyPressedDown || stickPressedDown)
	{
		m_cursorPosition++;
		if (m_cursorPosition > (int) (m_menuOptions.size()) - 1)
		{
			m_cursorPosition = 0;
		}
	}

	bool keyPressedUp = input->WasKeyJustPressed(InputSystem::KEYBOARD_UP_ARROW);
	bool stickPressedUp = (controller.WasStickJustPressed(XBOX_STICK_LEFT) && controller.GetCardinalStickDirection(XBOX_STICK_LEFT) == IntVector2::STEP_NORTH);

	if (keyPressedUp || stickPressedUp)
	{
		m_cursorPosition--;
		if (m_cursorPosition < 0)
		{
			m_cursorPosition = (int) (m_menuOptions.size()) - 1;
		}
	}

	if (input->WasKeyJustPressed(InputSystem::KEYBOARD_SPACEBAR) || controller.WasButtonJustPressed(XBOX_BUTTON_A))
	{
		ProcessMenuSelection();
	}

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


	renderer->BindTexture(0, "White");
	renderer->DisableDepth();

	renderer->DrawAABB2(m_menuBounds, AABB2::UNIT_SQUARE_OFFCENTER, Rgba::GREEN);

	// Draw the menu options
	BitmapFont* font = renderer->CreateOrGetBitmapFont("SquirrelFixedFont");
	AABB2 currentTextBounds = m_menuBounds;

	for (int menuIndex = 0; menuIndex < static_cast<int>(m_menuOptions.size()); ++menuIndex)
	{
		Rgba color = Rgba::WHITE;
		if (menuIndex == m_cursorPosition)
		{
			color = Rgba::YELLOW;
		}
		renderer->DrawTextInBox2D(m_menuOptions[menuIndex].m_displayText.c_str(), currentTextBounds, Vector2(0.5f, 0.f), m_fontHeight, TEXT_DRAW_SHRINK_TO_FIT, font, color);
		currentTextBounds.Translate(Vector2(0.f, -m_fontHeight));
	}

	renderer->EnableDepth(COMPARE_LESS, true);
}


void GameState_MainMenu::Enter()
{

}

void GameState_MainMenu::Leave()
{

}

void GameState_MainMenu::CreateMenuFromLoadedDefinitions()
{
	const std::map<std::string, BoardStateDefinition*>& definitions = BoardStateDefinition::GetAllDefinitions();

	std::map<std::string, BoardStateDefinition*>::const_iterator itr = definitions.begin();
	for (itr; itr != definitions.end(); ++itr)
	{
		m_menuOptions.push_back(MainMenuOption("Play: " + itr->first, itr->first));
		
	}

	// Push a quit option always
	m_menuOptions.push_back(MainMenuOption("Quit", ""));
}

//-----------------------------------------------------------------------------------------------
// Processes the enter command on a menu selection
//
void GameState_MainMenu::ProcessMenuSelection() const
{
	MainMenuOption selectedOption = m_menuOptions[m_cursorPosition];

	if (selectedOption.m_displayText == "Quit")
	{
		App::GetInstance()->Quit();
	}
	else
	{
		BoardStateDefinition* definition = BoardStateDefinition::GetDefinition(selectedOption.m_boardStateDefinitionName);
		GameState_Playing* newPlayState = new GameState_Playing(definition);
		Game::TransitionToGameState(newPlayState);
	}
}
