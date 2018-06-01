/************************************************************************/
/* File: PlayMode_Menu.hpp
/* Author: Andrew Chase
/* Date: March 12th, 2018
/* Description: Mode for handling Actor turn menu state and input
/************************************************************************/
#pragma once
#include <functional>
#include "Game/Actor.hpp"
#include "Game/Menu.hpp"
#include "Game/PlayMode.hpp"


class PlayMode_Menu : public PlayMode
{
public:
	//-----Public Methods-----

	PlayMode_Menu(ActorController* controller, std::vector<MenuOption_t>& menuOptions);

	// Virtual Methods
	virtual void ProcessInput() override;
	virtual void Update() override;
	virtual void RenderWorldSpace() const override;
	virtual void RenderScreenSpace() const override;
	virtual void Exit() override;


private:
	//-----Private Data-----

	Menu* m_menu;	// Menu with options, is rendered in screen space

};
