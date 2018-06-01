/************************************************************************/
/* File: Menu.hpp
/* Author: Andrew Chase
/* Date: March 12th, 2017
/* Description: Class to represent a UI menu for Actor actions
/************************************************************************/
#pragma once
#include "Game/PlayAction.hpp"
#include "Engine/Math/AABB2.hpp"
#include <string>
#include <vector>

// Struct to represent a single selectable menu option
struct MenuOption_t
{
	MenuOption_t() {};
	MenuOption_t(const std::string& text, eActionName actorAction, eActionClass type, bool isActive)
		: m_text(text), m_actionName(actorAction), m_actionClass(type), m_isActive(isActive)
	{}

	std::string		m_text;
	eActionName	m_actionName;
	eActionClass	m_actionClass;
	bool			m_isActive;
};

enum eMenuSelection
{
	MENU_NONE,
	MENU_SELECTED,
	MENU_CANCELLED
};


class Menu
{
public:
	//-----Public Methods-----

	Menu(std::vector<MenuOption_t>& options);

	eMenuSelection	ProcessInput();
	void			Update();
	void			Render() const;

	MenuOption_t	GetHighlightedOption() const;
	
	static std::vector<MenuOption_t> MakeMenuForActor(ActorController* controller);


private:
	//-----Private Methods-----

	void ResetHighlightIndex();
	void FindNextOptionUnderneath();
	void FindNextOptionAbove();

	static MenuOption_t MakeMenuOption(eActionName action, bool isActive);


private:
	//-----Private Data-----

	int							m_highlightIndex;
	std::vector<MenuOption_t>	m_options;

	// For rendering
	AABB2						m_menuBounds;
};
