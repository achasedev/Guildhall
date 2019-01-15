/************************************************************************/
/* File: Menu.cpp
/* Author: Andrew Chase
/* Date: November 24th 2018
/* Description: Implementation of the Menu class
/************************************************************************/
#include "Game/Framework/Menu.hpp"
#include "Game/GameStates/GameState_MainMenu.hpp"
#include "Engine/Input/InputSystem.hpp"


//-----------------------------------------------------------------------------------------------
// Constructor
//
Menu::Menu(GameState_MainMenu* mainMenu)
	: m_mainMenu(mainMenu)
{
	// Get sounds
	AudioSystem* audio = AudioSystem::GetInstance();

	m_cursorSound = audio->CreateOrGetSound("Data/Audio/SFX/Menu/sfx_sounds_interaction7.wav");
	m_confirmSound = audio->CreateOrGetSound("Data/Audio/SFX/Menu/sfx_sounds_damage3.wav");
	m_returnSound = audio->CreateOrGetSound("Data/Audio/SFX/Menu/sfx_sounds_damage3.wav");
}


//-----------------------------------------------------------------------------------------------
// Adds the given option to the end of the menu list
//
void Menu::AddOption(const std::string text, bool isSelectable, MenuOption_cb callback, const std::string& args)
{
	MenuOption_t option;
	option.text = text;
	option.isSelectable = isSelectable;
	option.callback = callback;
	option.args = args;
	
	m_options.push_back(option);
}


//-----------------------------------------------------------------------------------------------
// Sets the option for what happens when left is pressed while on the menu
//
void Menu::SetLeftOption(MenuOption_cb callback, const std::string& args)
{
	m_leftOption.callback = callback;
	m_leftOption.args = args;
}


//-----------------------------------------------------------------------------------------------
// Sets the option for what happens when right is pressed while on the menu
//
void Menu::SetRightOption(MenuOption_cb callback, const std::string& args)
{
	m_rightOption.callback = callback;
	m_rightOption.args = args;
}


//-----------------------------------------------------------------------------------------------
// Checks for menu input
//
void Menu::ProcessInput()
{
	InputSystem* input = InputSystem::GetInstance();

	// Moving down
	bool keyPressedDown = input->WasKeyJustPressed(InputSystem::KEYBOARD_DOWN_ARROW);
	if (keyPressedDown)
	{
		bool done = false;
		int oldPosition = m_cursorPosition;

		while (!done)
		{
			m_cursorPosition++;

			if (m_cursorPosition > (int)(m_options.size()) - 1)
			{
				m_cursorPosition = 0;
			}

			if (m_options[m_cursorPosition].isSelectable)
			{
				done = true;
			}
		}

		if (oldPosition != m_cursorPosition)
		{
			AudioSystem::GetInstance()->PlaySound(m_cursorSound, false, 0.2f);
		}
	}

	// Moving up
	bool keyPressedUp = input->WasKeyJustPressed(InputSystem::KEYBOARD_UP_ARROW);
	if (keyPressedUp)
	{
		bool done = false;
		int oldPosition = m_cursorPosition;

		while (!done)
		{
			m_cursorPosition--;

			if (m_cursorPosition < 0)
			{
				m_cursorPosition = (int)(m_options.size()) - 1;
			}

			if (m_options[m_cursorPosition].isSelectable)
			{
				done = true;
			}
		}

		if (oldPosition != m_cursorPosition)
		{
			AudioSystem::GetInstance()->PlaySound(m_cursorSound, false, 0.2f);
		}
	}

	// Moving left
	bool keyPressedLeft = input->WasKeyJustPressed(InputSystem::KEYBOARD_LEFT_ARROW);
	if (keyPressedLeft && m_leftOption.callback != nullptr)
	{
		m_leftOption.callback(m_mainMenu, m_leftOption.args);
	}

	// Moving right
	bool keyPressedRight = input->WasKeyJustPressed(InputSystem::KEYBOARD_RIGHT_ARROW);
	if (keyPressedRight && m_rightOption.callback != nullptr)
	{
		m_rightOption.callback(m_mainMenu, m_rightOption.args);
	}

	// Selection
	if (input->WasKeyJustPressed(InputSystem::KEYBOARD_SPACEBAR))
	{
		AudioSystem::GetInstance()->PlaySound(m_confirmSound);

		ProcessCurrentMenuSelection();
	}
}


//-----------------------------------------------------------------------------------------------
// Sets the cursor position to the index given
//
void Menu::SetCursorPosition(int cursorPosition)
{
	m_cursorPosition = cursorPosition;
}


//-----------------------------------------------------------------------------------------------
// Returns the current cursor index
//
int Menu::GetCursorPosition() const
{
	return m_cursorPosition;
}


//-----------------------------------------------------------------------------------------------
// Returns a list of options texts used for rendering
//
std::vector<std::string> Menu::GetTextsForRender() const
{
	std::vector<std::string> texts;

	for (int i = 0; i < (int)m_options.size(); ++i)
	{
		texts.push_back(m_options[i].text);
	}

	return texts;
}


//-----------------------------------------------------------------------------------------------
// Calls the callback on the currently selected menu option
//
void Menu::ProcessCurrentMenuSelection()
{
	m_options[m_cursorPosition].callback(m_mainMenu, m_options[m_cursorPosition].args);
}
