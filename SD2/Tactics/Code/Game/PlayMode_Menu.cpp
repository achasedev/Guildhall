/************************************************************************/
/* File: PlayMode_Menu.cpp
/* Author: Andrew Chase
/* Date: March 12th, 2018
/* Description: Implementation for the PlayMode_Menu class
/************************************************************************/
#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"
#include "Game/PlayMode_Menu.hpp"
#include "Game/ActorController.hpp"
#include "Game/GameState_Playing.hpp"

#include "Engine/Renderer/Renderer.hpp"


//-----------------------------------------------------------------------------------------------
// Single constructor, takes the controller for the current actor and a list of options for the menu
//
PlayMode_Menu::PlayMode_Menu(ActorController* controller, std::vector<MenuOption_t>& menuOptions)
	: PlayMode(controller)
{
	// Make a new menu given the options
	m_menu = new Menu(menuOptions);
}


//-----------------------------------------------------------------------------------------------
// Checks for input on the menu (moving the highlighted index, selection)
//
void PlayMode_Menu::ProcessInput()
{
	eMenuSelection menuSelection = m_menu->ProcessInput();

	// If the menu selected something, call the callback
	if (menuSelection == MENU_SELECTED)
	{
		MenuOption_t selectedOption = m_menu->GetHighlightedOption();
		m_controller->OnMenuSelection(false, selectedOption.m_actionName);
		m_isFinished = true;
	}
	else if (menuSelection == MENU_CANCELLED)
	{
		// Pushes an ObserveMode, will eventually return to this mode
		m_controller->OnMenuSelection(true, ACTION_NAME_NONE);
	}
}


//-----------------------------------------------------------------------------------------------
// Update
//
void PlayMode_Menu::Update()
{
	// Update the camera
	GameState_Playing* playState = Game::GetGameStatePlaying();
	playState->TranslateCamera(m_controller->GetActor()->GetWorldPosition());
}


//-----------------------------------------------------------------------------------------------
// Renders world space
//
void PlayMode_Menu::RenderWorldSpace() const
{
	// Render the cursor (should be under the actor)
	PlayMode::RenderWorldSpace();
}


//-----------------------------------------------------------------------------------------------
// Renders screen space
//
void PlayMode_Menu::RenderScreenSpace() const
{
	// Render the current actor's menu
	m_menu->Render();
	PlayMode::RenderScreenSpace();
}


//-----------------------------------------------------------------------------------------------
// Called before deletion
//
void PlayMode_Menu::Exit()
{
}
