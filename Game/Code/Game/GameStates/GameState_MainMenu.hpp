/************************************************************************/
/* File: GameState_MainMenu.hpp
/* Author: Andrew Chase
/* Date: March 24th, 2018
/* Description: Class to represent the state when the Game is at the main menu
/************************************************************************/
#pragma once
#include <string>
#include <vector>
#include "Game/GameStates/GameState.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/IntVector3.hpp"
#include "Engine/Core/Time/Stopwatch.hpp"

class VoxelFont;
class World;
class VoxelEmitter;
class Menu;

enum eSubMenu
{
	SUB_MENU_MAIN,
	SUB_MENU_CAMPAIGNS,
	SUB_MENU_LEADERBOARD_1,
	SUB_MENU_LEADERBOARD_2, 
	SUB_MENU_LEADERBOARD_3, 
	SUB_MENU_LEADERBOARD_4
};

class GameState_MainMenu : public GameState
{
public:
	//-----Public Methods-----
	
	GameState_MainMenu();
	~GameState_MainMenu();

	virtual void ProcessInput() override;

	virtual bool Enter() override;
	virtual void Update() override;
	virtual bool Leave() override;

	virtual void Render_Enter() const override;
	virtual void Render() const override;
	virtual void Render_Leave() const override;

	void MoveToSubMenu(eSubMenu subMenu);


private:
	//-----Private Methods-----

	void DrawCurrentMenu() const;
	void ParticalizeCurrentMenu() const;


private:
	//-----Private Data-----

	Menu*						m_currentMenu = nullptr;

	IntVector3					m_menuStartCoord = IntVector3(128, 20, 160);
	Vector3						m_defaultCameraPosition = Vector3(128.f, 100.f, 35.f);

	VoxelEmitter* m_emitters[2];

};
