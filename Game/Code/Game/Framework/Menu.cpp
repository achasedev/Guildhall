/************************************************************************/
/* File: Menu.cpp
/* Author: Andrew Chase
/* Date: November 24th 2018
/* Description: Implementation of the Menu class
/************************************************************************/
#include "Game/Framework/Menu.hpp"
#include "Game/Framework/GameAudioSystem.hpp"
#include "Game/GameStates/GameState_MainMenu.hpp"
#include "Engine/Input/InputSystem.hpp"


//-----------------------------------------------------------------------------------------------
// Constructor
//
Menu::Menu(GameState_MainMenu* mainMenu)
	: m_mainMenu(mainMenu)
{
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
// Sets the option for when "UP" is pressed with the menu active
// WILL OVERRIDE ALL CURSOR INCREMENTS
//
void Menu::SetUpOption(MenuOption_cb callback, const std::string& args)
{
	m_upOption.callback = callback;
	m_upOption.args = args;
}


//-----------------------------------------------------------------------------------------------
// Sets the option for when "DOWN" is pressed with the menu active
// WILL OVERRIDE ALL CURSOR INCREMENTS
//
void Menu::SetDownOption(MenuOption_cb callback, const std::string& args)
{
	m_downOption.callback = callback;
	m_downOption.args = args;
}


//-----------------------------------------------------------------------------------------------
// Checks for menu input
//
void Menu::ProcessInput()
{
	XboxController& cont = InputSystem::GetPlayerOneController();
	InputSystem* input = InputSystem::GetInstance();

	// Down input
	bool keyPressedDown = input->WasKeyJustPressed(InputSystem::KEYBOARD_DOWN_ARROW);
	bool contPressedDown = cont.WasStickJustPressed(XBOX_STICK_LEFT) && cont.GetCorrectedStickPosition(XBOX_STICK_LEFT).y < 0.f;
	contPressedDown = contPressedDown || cont.WasButtonJustPressed(XBOX_BUTTON_DPAD_DOWN);

	if (keyPressedDown || contPressedDown)
	{
		// Check if we have an override for down first
		if (m_downOption.callback != nullptr)
		{
			m_downOption.callback(m_mainMenu, m_downOption.args);
		}
		else
		{
			// Else move the cursor, looking for the next available slot
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
				Game::GetGameAudioSystem()->PlaySystemSound("Menu_cursor");
			}
		}
	}

	// Up input
	bool keyPressedUp = input->WasKeyJustPressed(InputSystem::KEYBOARD_UP_ARROW);
	bool contPressedUp = cont.WasStickJustPressed(XBOX_STICK_LEFT) && cont.GetCorrectedStickPosition(XBOX_STICK_LEFT).y > 0.f;
	contPressedUp = contPressedUp || cont.WasButtonJustPressed(XBOX_BUTTON_DPAD_UP);

	if (keyPressedUp || contPressedUp)
	{
		// Check for override
		if (m_upOption.callback != nullptr)
		{
			m_upOption.callback(m_mainMenu, m_upOption.args);
		}
		else
		{
			// Else move the cursor up, looking for a valid position
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
				Game::GetGameAudioSystem()->PlaySystemSound("Menu_cursor");
			}
		}
	}

	// Moving left
	bool keyPressedLeft = input->WasKeyJustPressed(InputSystem::KEYBOARD_LEFT_ARROW);
	bool contPressedLeft = cont.WasStickJustPressed(XBOX_STICK_LEFT) && cont.GetCorrectedStickPosition(XBOX_STICK_LEFT).x < 0.f;
	contPressedLeft = contPressedLeft || cont.WasButtonJustPressed(XBOX_BUTTON_DPAD_LEFT);

	if ((keyPressedLeft || contPressedLeft) && m_leftOption.callback != nullptr)
	{
		m_leftOption.callback(m_mainMenu, m_leftOption.args);
	}

	// Moving right
	bool keyPressedRight = input->WasKeyJustPressed(InputSystem::KEYBOARD_RIGHT_ARROW);
	bool contPressedRight = cont.WasStickJustPressed(XBOX_STICK_LEFT) && cont.GetCorrectedStickPosition(XBOX_STICK_LEFT).x > 0.f;
	contPressedRight = contPressedRight || cont.WasButtonJustPressed(XBOX_BUTTON_DPAD_RIGHT);

	if ((keyPressedRight || contPressedRight) && m_rightOption.callback != nullptr)
	{
		m_rightOption.callback(m_mainMenu, m_rightOption.args);
	}

	// Selection
	if (input->WasKeyJustPressed(InputSystem::KEYBOARD_SPACEBAR) || cont.WasButtonJustPressed(XBOX_BUTTON_START) || cont.WasButtonJustPressed(XBOX_BUTTON_A))
	{
		Game::GetGameAudioSystem()->PlaySystemSound("Menu_confirm");

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
