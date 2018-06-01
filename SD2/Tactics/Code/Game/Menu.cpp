/************************************************************************/
/* File: Menu.cpp
/* Author: Andrew Chase
/* Date: February 22nd, 2017
/* Description: Implementation of the Menu class
/************************************************************************/
#include "Game/Menu.hpp"
#include "Game/Actor.hpp"
#include "Game/ActorController.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Core/Window.hpp"
#include "Engine/Renderer/Renderer.hpp"


//-----------------------------------------------------------------------------------------------
// Constructor
//
Menu::Menu(std::vector<MenuOption_t>& options)
	: m_options(options)
	, m_highlightIndex(-1)
{
	m_menuBounds = AABB2(Vector2(50.f, 50.f), Vector2(Window::GetInstance()->GetWindowAspect() * 200.f, 250.f));

	// Place the cursor as high up on the list as possible
	FindNextOptionUnderneath();
}


//-----------------------------------------------------------------------------------------------
// Checks for input on this menu, and if it processes input, returns true to consume it
// Else returns false
//
eMenuSelection Menu::ProcessInput()
{
	InputSystem* input			= InputSystem::GetInstance();
	XboxController& controller	= InputSystem::GetPlayerOneController();

	if (controller.WasStickJustPressed(XBOX_STICK_LEFT))
	{
		IntVector2 inputDirection = IntVector2(controller.GetCardinalStickDirection(XBOX_STICK_LEFT));

		if (inputDirection == IntVector2::STEP_NORTH)
		{
			FindNextOptionAbove();
		}

		if (inputDirection == IntVector2::STEP_SOUTH)
		{
			FindNextOptionUnderneath();
		}
	}
	else if (input->WasKeyJustPressed(InputSystem::KEYBOARD_UP_ARROW))
	{
		FindNextOptionAbove();
	}
	else if (input->WasKeyJustPressed(InputSystem::KEYBOARD_DOWN_ARROW))
	{
		FindNextOptionUnderneath();
	}


	// Confirm button
	if (controller.WasButtonJustPressed(XBOX_BUTTON_A) || input->WasKeyJustPressed(InputSystem::KEYBOARD_SPACEBAR))
	{
		return MENU_SELECTED;
	}
	

	// Cancel button
	if (controller.WasButtonJustPressed(XBOX_BUTTON_B))
	{
		return MENU_CANCELLED;
	}

	return MENU_NONE;
}


//-----------------------------------------------------------------------------------------------
// Updates the Menu
//
void Menu::Update()
{
}


//-----------------------------------------------------------------------------------------------
// Draw the Menu to screen (in screen space)
//
void Menu::Render() const
{
	Renderer* renderer = Renderer::GetInstance();

	renderer->SetCurrentCamera(renderer->GetUICamera());
	renderer->DisableDepth();
	renderer->BindTexture(0, "White");
	renderer->DrawAABB2(m_menuBounds, AABB2::UNIT_SQUARE_OFFCENTER, Rgba(0,0,150,200));

	BitmapFont* font = renderer->CreateOrGetBitmapFont("SquirrelFixedFont");
	AABB2 currTextBounds = m_menuBounds;
	for (int optionIndex = 0; optionIndex < (int) m_options.size(); optionIndex++)
	{
		MenuOption_t currOption = m_options[optionIndex];

		Rgba color = Rgba::WHITE;
		if (!currOption.m_isActive)
		{
			color = Rgba(100, 100, 100, 255);
		}
		
		if (optionIndex == m_highlightIndex)
		{
			color = Rgba::LIGHT_GREEN;
		}

		renderer->DrawTextInBox2D(currOption.m_text, currTextBounds, Vector2(0.5f, 0.f), 50.f, TEXT_DRAW_OVERRUN, font, color);

		currTextBounds.Translate(Vector2(0.f, -50.f));
	}
}


//-----------------------------------------------------------------------------------------------
// Returns the Menu option currently at the highlighted option index
//
MenuOption_t Menu::GetHighlightedOption() const
{
	return m_options[m_highlightIndex];
}


//-----------------------------------------------------------------------------------------------
// STATIC - Makes a list of options the given controller (actor within the controller) can use
//
std::vector<MenuOption_t> Menu::MakeMenuForActor(ActorController* controller)
{
	Actor* actor = controller->GetActor();

	std::vector<eActionName> actions = actor->GetActions();
	std::vector<MenuOption_t> options;

	for (int actionIndex = 0; actionIndex < (int) actions.size(); ++actionIndex)
	{
		eActionName actionName = actions[actionIndex];
		eActionClass actionClass = PlayAction::GetClassFromName(actionName);
		bool isActive = !controller->WasActionClassUsed(actionClass);

		options.push_back(MakeMenuOption(actions[actionIndex], isActive));
	}

	return options;
}


//-----------------------------------------------------------------------------------------------
// Constructs a MenuOption of the given name and returns it
//
MenuOption_t Menu::MakeMenuOption(eActionName action, bool isActive)
{
	MenuOption_t option;
	option.m_actionName = action;
	option.m_isActive = isActive;

	switch (action)
	{
	case ACTION_NAME_ATTACK:
		option.m_actionClass = ACTION_CLASS_ATTACK;
		option.m_text = "Attack";
		break;
	case ACTION_NAME_MOVE:
		option.m_actionClass = ACTION_CLASS_MOVE;
		option.m_text = "Move";
		break;
	case ACTION_NAME_ARROW:
		option.m_actionClass = ACTION_CLASS_ATTACK;
		option.m_text = "Arrow";
		break;
	case ACTION_NAME_WAIT:
		option.m_actionClass = ACTION_CLASS_WAIT;
		option.m_text = "Wait";
		break;
	case ACTION_NAME_METEOR:
		option.m_actionClass = ACTION_CLASS_ATTACK;
		option.m_text = "Meteor";
		break;
	case ACTION_NAME_GUARD:
		option.m_actionClass = ACTION_CLASS_ATTACK;
		option.m_text = "Guard";
		break;
	case ACTION_NAME_HEAL:
		option.m_actionClass = ACTION_CLASS_ATTACK;
		option.m_text = "Heal";
		break;
	default:
		ERROR_AND_DIE("Menu::MakeMenuOption() received bad action.");
	}

	return option;
}

//-----------------------------------------------------------------------------------------------
// Resets the highlight index to the first open option index (guaranteed one will exist)
//
void Menu::ResetHighlightIndex()
{
	for (int index = 0; index < (int) m_options.size(); ++index)
	{
		if (m_options[index].m_isActive)
		{
			m_highlightIndex = index;
			break;
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Finds the next active option index greater than the highlight index and sets the highlight
// index to it, or leaves it unchanged if no such option index exist
//
void Menu::FindNextOptionUnderneath()
{
	for (int index = m_highlightIndex + 1; index < (int) m_options.size(); index++)
	{
		if (m_options[index].m_isActive)
		{
			m_highlightIndex = index;
			return;
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Finds the next active option index less than the highlight index and sets the highlight
// index to it, or leaves it unchanged if no such option index exist
//
void Menu::FindNextOptionAbove()
{
	for (int index = m_highlightIndex - 1; index >= 0; index--)
	{
		if (m_options[index].m_isActive)
		{
			m_highlightIndex = index;
			return;
		}
	}
}
