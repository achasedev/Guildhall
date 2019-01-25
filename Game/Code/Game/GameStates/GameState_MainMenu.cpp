/************************************************************************/
/* File: GameState_MainMenu.cpp
/* Author: Andrew Chase
/* Date: March 24th, 2018
/* Description: Implementation of the GameState_MainMenu class
/************************************************************************/
#include "Game/Framework/App.hpp"
#include "Game/Framework/Game.hpp"
#include "Game/Framework/GameCommon.hpp"
#include "Game/GameStates/GameState_Playing.hpp"
#include "Game/GameStates/GameState_MainMenu.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Rendering/DebugRendering/DebugRenderSystem.hpp"

//-----------------------------------------------------------------------------------------------
// Default constructor
//
GameState_MainMenu::GameState_MainMenu()
	: GameState(1.f, 1.f)
{
}


//-----------------------------------------------------------------------------------------------
// Destructor
//
GameState_MainMenu::~GameState_MainMenu()
{
}


//-----------------------------------------------------------------------------------------------
// Checks for input related to this GameState and changes state accordingly
//
void GameState_MainMenu::ProcessInput()
{
	if (InputSystem::GetInstance()->WasKeyJustPressed(' '))
	{
		Game::TransitionToGameState(new GameState_Playing());
	}
}


//-----------------------------------------------------------------------------------------------
// Updates the MainMenu state
//
void GameState_MainMenu::Update()
{
}


//-----------------------------------------------------------------------------------------------
// Renders the Main Menu
//
void GameState_MainMenu::Render() const
{
	DebugRenderSystem::GetInstance()->Draw2DText("Main Menu - Press Space", AABB2(Vector2(0.f, 0.f), Vector2(200.f, 200.f)), 0.f, Rgba::WHITE, 50.f, Vector2(0.f, 1.0f));
}


//-----------------------------------------------------------------------------------------------
// Renders the leave state
//
void GameState_MainMenu::Render_Leave() const
{
	DebugRenderSystem::GetInstance()->Draw2DText(Stringf("Main Menu - Leaving in %f", m_transitionTimer.GetTimeUntilIntervalEnds()), AABB2(Vector2(0.f, 0.f), Vector2(200.f, 200.f)), 0.f, Rgba::WHITE, 50.f, Vector2(0.f, 1.0f));
}


//-----------------------------------------------------------------------------------------------
// Called when the game transitions into this state, before the first running update
//
bool GameState_MainMenu::Enter()
{
	return m_transitionTimer.HasIntervalElapsed();
}


//-----------------------------------------------------------------------------------------------
// Called when the game transitions out of this state, before deletion
//
bool GameState_MainMenu::Leave()
{
	return m_transitionTimer.HasIntervalElapsed();
}


//-----------------------------------------------------------------------------------------------
// Renders the enter state
//
void GameState_MainMenu::Render_Enter() const
{
	DebugRenderSystem::GetInstance()->Draw2DText(Stringf("Main Menu - Entering in %f", m_transitionTimer.GetTimeUntilIntervalEnds()), AABB2(Vector2(0.f, 0.f), Vector2(200.f, 200.f)), 0.f, Rgba::WHITE, 50.f, Vector2(0.f, 1.0f));
}
