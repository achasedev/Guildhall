/************************************************************************/
/* File: GameState_MainMenu.hpp
/* Author: Andrew Chase
/* Date: March 24th, 2018
/* Description: Class to represent the state when the Game is at the main menu
/************************************************************************/
#pragma once
#include <vector>
#include "Game/GameStates/GameState.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/IntVector3.hpp"

class VoxelFont;
class World;
class VoxelEmitter;

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


private:
	//-----Private Methods-----

	void ProcessMenuSelection() const;


private:
	//-----Private Data-----

	std::vector<std::string>	m_menuOptions;
	int							m_cursorPosition;

	IntVector3					m_menuStartCoord = IntVector3(128, 8, 160);
	VoxelFont*					m_menuFont;
	Vector3						m_defaultCameraPosition = Vector3(128.f, 100.f, 35.f);

	VoxelEmitter* m_emitters[2];

};
