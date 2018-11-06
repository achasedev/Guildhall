/************************************************************************/
/* File: GameState_Ready.cpp
/* Author: Andrew Chase
/* Date: June 1st, 2018
/* Description: Implementation of the Ready GameState class
/************************************************************************/
#include "Game/GameStates/GameState_Ready.hpp"
#include "Game/GameStates/GameState_Playing.hpp"

#include "Engine/Core/Window.hpp"
#include "Engine/Assets/AssetDB.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Rendering/Core/Renderer.hpp"


//-----------------------------------------------------------------------------------------------
// Constructor
//
GameState_Ready::GameState_Ready()
	: GameState(0.f, 0.f)
{
}


//-----------------------------------------------------------------------------------------------
// Checks for input
//
void GameState_Ready::ProcessInput()
{
	InputSystem* input = InputSystem::GetInstance();

	if (input->WasKeyJustPressed(' '))
	{
		Game::TransitionToGameState(new GameState_Playing());
	}
}


//-----------------------------------------------------------------------------------------------
// Simulation update
//
void GameState_Ready::Update()
{

}


//-----------------------------------------------------------------------------------------------
// Draws the ready UI elements to screen
//
void GameState_Ready::Render() const
{
	Renderer* renderer = Renderer::GetInstance();
	renderer->SetCurrentCamera(renderer->GetUICamera());

	// Setup the renderer
	renderer->ClearScreen(Rgba::LIGHT_BLUE);
	renderer->Draw2DQuad(m_textBoxBounds, AABB2::UNIT_SQUARE_OFFCENTER, Rgba::BLUE, AssetDB::GetSharedMaterial("UI"));

	// Draw the text
	BitmapFont* font = AssetDB::CreateOrGetBitmapFont("Data/Images/Fonts/Default.png");
	renderer->DrawTextInBox2D("In Ready state, press 'space' to play", m_textBoxBounds, Vector2(0.5f, 0.5f), 40.f, TEXT_DRAW_SHRINK_TO_FIT, font, Rgba::WHITE);
}


//-----------------------------------------------------------------------------------------------
// Renders the leave state transition
//
void GameState_Ready::Render_Leave() const
{
	Render();
}


//-----------------------------------------------------------------------------------------------
// Called when the game transitions into this state, before the first update
//
bool GameState_Ready::Enter()
{
	float aspect = Window::GetInstance()->GetAspect();
	float height = Renderer::UI_ORTHO_HEIGHT;

	m_textBoxBounds = AABB2(Vector2(0.1f * aspect * height, 0.1f * height), Vector2(0.9f * aspect * height, 0.4f * height));

	return true;
}


//-----------------------------------------------------------------------------------------------
// Called when the game transitions out of this state, before deletion
//
bool GameState_Ready::Leave()
{
	return true;
}


//-----------------------------------------------------------------------------------------------
// Renders the enter state
//
void GameState_Ready::Render_Enter() const
{
	Render();
}
