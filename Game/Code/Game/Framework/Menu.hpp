/************************************************************************/
/* File: Menu.hpp
/* Author: Andrew Chase
/* Date: November 24th 2018
/* Description: Class to represent a single menu for the main menu
/************************************************************************/
#pragma once
#include <string>
#include <vector>

class GameState_MainMenu;
typedef void(*MenuOption_cb)(GameState_MainMenu* mainMenu, const std::string& args);

struct MenuOption_t
{
	std::string text;
	bool isSelectable;
	MenuOption_cb callback;
	std::string args;
};


class Menu
{
public:
	//-----Public Methods-----
	
	Menu(GameState_MainMenu* mainMenu);

	void AddOption(const std::string text, bool isSelectable, MenuOption_cb callback, const std::string& args);

	void ProcessInput();

	bool Enter();
	void Update();
	bool Leave();

	void SetCursorPosition(int cursorPosition);
	int GetCursorPosition() const;
	std::vector<std::string> GetTextsForRender() const;


public:
	//-----Public Data-----
	
	void ProcessCurrentMenuSelection();


private:
	//-----Private Methods-----
	
	
private:
	//-----Private Data-----
	
	GameState_MainMenu* m_mainMenu;
	int m_cursorPosition = 0;
	std::vector<MenuOption_t> m_options;

};
