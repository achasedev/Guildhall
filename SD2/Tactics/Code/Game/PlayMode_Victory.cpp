/************************************************************************/
/* File: PlayMode_Victory.cpp
/* Author: Andrew Chase
/* Date: March 12th, 2018
/* Description: Implementation for the Victory PlayMode
/************************************************************************/
#include "Game/Game.hpp"
#include "Game/Actor.hpp"
#include "Game/BoardState.hpp"
#include "Game/PlayMode_Victory.hpp"
#include "Game/GameState_MainMenu.hpp"

#include "Engine/Core/Stopwatch.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Renderer/Renderer.hpp"
//-----------------------------------------------------------------------------------------------
// Constructor - does nothing (Victory mode doesn't need a controller)
//
PlayMode_Victory::PlayMode_Victory()
	: PlayMode(nullptr)
{
	m_stopwatch = new Stopwatch(Game::GetGameClock());
	m_stopwatch->SetTimer(2.0f);
}


//-----------------------------------------------------------------------------------------------
// Starts the stopwatch for the grayscale effect
//
void PlayMode_Victory::Start()
{
	m_isStarted = true;
}


//-----------------------------------------------------------------------------------------------
// Checks to exit the victory screen, returning to the main menu
//
void PlayMode_Victory::ProcessInput()
{
	XboxController& controller = InputSystem::GetPlayerOneController();

	if (controller.WasButtonJustPressed(XBOX_BUTTON_A))
	{
		Game::TransitionToGameState(new GameState_MainMenu());
	}
}


//-----------------------------------------------------------------------------------------------
// Update - for Victory effects
//
void PlayMode_Victory::Update()
{
}


//-----------------------------------------------------------------------------------------------
// Renders the world space
//
void PlayMode_Victory::RenderWorldSpace() const
{
}


//-----------------------------------------------------------------------------------------------
// Draws the victory overlay
//
void PlayMode_Victory::RenderScreenSpace() const
{
	Renderer* renderer = Renderer::GetInstance();

	// Setup and apply grayscale effect
	float fade = ClampFloatZeroToOne(m_stopwatch->GetElapsedTimeNormalized());
	renderer->SetCurrentShaderProgram("Grayscale");
	renderer->BindUniformFloat("LERP", fade);

	renderer->ApplyImageEffect("Grayscale");
	renderer->FinalizeImageEffects();

	// Return to normal state
	renderer->SetCurrentCamera(renderer->GetUICamera());
	renderer->DisableDepth();
	renderer->BindTexture(0, "White");

	BoardState* boardState = Game::GetCurrentBoardState();
	AABB2 bounds = Renderer::GetUIBounds();

	// Draw the Text
	renderer->DrawAABB2(bounds, AABB2::UNIT_SQUARE_OFFCENTER, Rgba(0,0,0,100));
	BitmapFont* font = renderer->CreateOrGetBitmapFont("SquirrelFixedFont");
	renderer->DrawTextInBox2D(Stringf("Team %i wins!\nPress Controller A to return to Main Menu.", boardState->GetActorByIndex(0)->GetTeamIndex()), bounds, Vector2(0.5f, 0.5f), 50.f, TEXT_DRAW_SHRINK_TO_FIT, font);
}


//-----------------------------------------------------------------------------------------------
// Called before deletion of this mode
//
void PlayMode_Victory::Exit()
{
}
