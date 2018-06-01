/************************************************************************/
/* File: GameState_MainMenu.hpp
/* Author: Andrew Chase
/* Date: February 12th, 2018
/* Description: Class to represent the state when the Game is at the main menu
/************************************************************************/
#pragma once
#include <vector>
#include "Game/GameState.hpp"
#include "Engine/Math/AABB2.hpp"

struct MainMenuOption
{
	MainMenuOption(const std::string& displayText, const std::string& boardStateDefinitionName)
		: m_displayText(displayText), m_boardStateDefinitionName(boardStateDefinitionName) {}
	std::string m_displayText;
	std::string m_boardStateDefinitionName;
};

class GameState_MainMenu : public GameState
{
public:
	//-----Public Methods-----
	
	GameState_MainMenu();

	virtual void ProcessInput() override;
	virtual void Update() override;
	virtual void Render() const override;

	virtual void Enter() override;
	virtual void Leave() override;


private:
	//-----Private Methods-----

	void CreateMenuFromLoadedDefinitions();
	void ProcessMenuSelection() const;


private:
	//-----Private Data-----

	std::vector<MainMenuOption>	m_menuOptions;
	int							m_cursorPosition;
	
	float m_fontHeight;
	AABB2 m_menuBounds;
};
