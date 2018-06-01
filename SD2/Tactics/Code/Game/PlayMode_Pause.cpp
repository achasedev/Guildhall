/************************************************************************/
/* File: PlayMode_Pause.cpp
/* Author: Andrew Chase
/* Date: March 20th, 2018
/* Description: Implementation of the PlayMode pause class
/************************************************************************/
#include "Game/Game.hpp"
#include "Game/PlayMode_Pause.hpp"
#include "Game/GameState_MainMenu.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/Window.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Stopwatch.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Renderer/OrbitCamera.hpp"

// Time from when button is pressed to pause menu fully faded in/faded out
const float PlayMode_Pause::PAUSE_FADE_TIME = 1.0f;


//-----------------------------------------------------------------------------------------------
// Constructor - Sets up draw bounds and stopwatch for rendering
// (Can't do this in start as there may be actions in the action queue, so start/update won't
// get called for a mode)
//
PlayMode_Pause::PlayMode_Pause()
	: PlayMode(nullptr)
{
	Renderer* renderer = Renderer::GetInstance();
	float uiHeight = renderer->UI_ORTHO_HEIGHT;
	float uiWidth = Window::GetInstance()->GetWindowAspect() * uiHeight;

	m_textBounds = AABB2(Vector2::ZERO, Vector2(uiWidth, uiHeight));
	m_textBounds.AddPaddingToSides(-100.f, -200.f);

	m_stopwatch = new Stopwatch(Clock::GetMasterClock());
	Game::GetGameClock()->SetPaused(true);
	m_stopwatch->SetTimer(PAUSE_FADE_TIME);
}


//-----------------------------------------------------------------------------------------------
// Checks if the pause button is pressed, and if so starts fading back into the game
//
void PlayMode_Pause::ProcessInput()
{
	InputSystem* input = InputSystem::GetInstance();
	XboxController& controller = InputSystem::GetPlayerOneController();

	if (m_fadingIn)
	{
		if (controller.WasButtonJustPressed(XBOX_BUTTON_START) || input->WasKeyJustPressed(InputSystem::KEYBOARD_F7))
		{
			// Make the fade into game duration = amount of time passed, for smooth transition if we're still fading
			float timeElapsed = ClampFloat(m_stopwatch->GetElapsedTime(), 0.f, PAUSE_FADE_TIME);
			m_stopwatch->SetTimer(PAUSE_FADE_TIME);
			m_stopwatch->SetElapsedTime(PAUSE_FADE_TIME - timeElapsed);
			m_fadingIn = false;
		}
		else if (controller.WasButtonJustPressed(XBOX_BUTTON_BACK) || input->WasKeyJustPressed(InputSystem::KEYBOARD_ESCAPE))
		{
			// Quit to the main menu
			Game::GetGameClock()->SetPaused(false);
			Game::GetInstance()->TransitionToGameState(new GameState_MainMenu());
		}
	}
	else if (m_stopwatch->HasIntervalElapsed())
	{
		// Check if we're done
		Game::GetGameClock()->SetPaused(false);
		m_isFinished = true;
	}
}


//-----------------------------------------------------------------------------------------------
// UNUSED
//
void PlayMode_Pause::Update()
{
}


//-----------------------------------------------------------------------------------------------
// For rendering world items
//
void PlayMode_Pause::RenderWorldSpace() const
{
}


//-----------------------------------------------------------------------------------------------
// Applies the grayscale effect and draws color text overtop
//
void PlayMode_Pause::RenderScreenSpace() const
{
	Renderer* renderer = Renderer::GetInstance();
	BitmapFont* font = renderer->CreateOrGetBitmapFont("SquirrelFixedFont");

	float fade = ClampFloatZeroToOne(m_stopwatch->GetElapsedTimeNormalized());
	if (!m_fadingIn)
	{
		fade = 1.0f - fade;
	}

	renderer->SetCurrentShaderProgram("Grayscale");
	renderer->BindUniformFloat("LERP", fade);

	renderer->ApplyImageEffect("Grayscale");
	renderer->FinalizeImageEffects();
	renderer->SetCurrentCamera(renderer->GetUICamera());
	renderer->DisableDepth();

	// Draw pause text
	AABB2 screenBounds = Window::GetInstance()->GetWindowBounds();

	Rgba textColor = Rgba::YELLOW;
	textColor.ScaleAlpha(fade);

	renderer->DrawTextInBox2D("Game Paused\nPress Start to resume,\nor Back to return to the main menu", m_textBounds, Vector2(0.5f, 0.5f), 30.f, TEXT_DRAW_SHRINK_TO_FIT, font, textColor);
}


//-----------------------------------------------------------------------------------------------
// Called before deletion
//
void PlayMode_Pause::Exit()
{
}
