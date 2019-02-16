/************************************************************************/
/* File: PlayState_Pause.cpp
/* Author: Andrew Chase
/* Date: January 7th 2018
/* Description: Implementation of the Pause PlayState
/************************************************************************/
#include "Game/Entity/Player.hpp"
#include "Game/Framework/Game.hpp"
#include "Game/Framework/World.hpp"
#include "Game/Framework/VoxelFont.hpp"
#include "Game/GameStates/GameState_Playing.hpp"
#include "Game/GameStates/GameState_MainMenu.hpp"
#include "Game/Framework/PlayStates/PlayState_Pause.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Time/Clock.hpp"
#include "Engine/Input/InputSystem.hpp"

//-----------------------------------------------------------------------------------------------
// Constructor
//
PlayState_Pause::PlayState_Pause(Player* pausingPlayer)
	: PlayState(PAUSE_TRANSITION_IN_TIME, PAUSE_TRANSITION_OUT_TIME)
	, m_playerThatPaused(pausingPlayer)
{
	m_menuText.push_back(Stringf("Player %i", pausingPlayer->GetPlayerID() + 1));
	m_menuText.push_back("Paused");
	m_menuText.push_back("Resume");
	m_menuText.push_back("Quit");
}


//-----------------------------------------------------------------------------------------------
// Destructor
//
PlayState_Pause::~PlayState_Pause()
{
}


//-----------------------------------------------------------------------------------------------
// Checks for input to return to the main menu
//
void PlayState_Pause::ProcessInput()
{
	InputSystem* input = InputSystem::GetInstance();
	XboxController& controller = input->GetController(m_playerThatPaused->GetPlayerID());

	bool confirmPressed = input->WasKeyJustPressed(' ') || controller.WasButtonJustPressed(XBOX_BUTTON_START) || controller.WasButtonJustPressed(XBOX_BUTTON_A);

	if (confirmPressed)
	{
		ProcessSelection();
	}
	else
	{
		bool upPressed = input->WasKeyJustPressed(InputSystem::KEYBOARD_UP_ARROW) || controller.WasButtonJustPressed(XBOX_BUTTON_DPAD_UP) ||
			(controller.WasStickJustPressed(XBOX_STICK_LEFT) && controller.GetCorrectedStickPosition(XBOX_STICK_LEFT).y > 0.f);

		bool downPressed = input->WasKeyJustPressed(InputSystem::KEYBOARD_DOWN_ARROW) || controller.WasButtonJustPressed(XBOX_BUTTON_DPAD_DOWN) ||
			(controller.WasStickJustPressed(XBOX_STICK_LEFT) && controller.GetCorrectedStickPosition(XBOX_STICK_LEFT).y < 0.f);

		if (upPressed)
		{
			m_cursorIndex--;
			if (m_cursorIndex < 2)
			{
				m_cursorIndex = (int) m_menuText.size() - 1;
			}
		}

		if (downPressed)
		{
			m_cursorIndex++;
			if (m_cursorIndex >= m_menuText.size())
			{
				m_cursorIndex = 2; // Skip the Player and "Paused" text
			}
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Enter update
//
bool PlayState_Pause::Enter()
{
	static bool pauseEntered = false;

	if (!pauseEntered)
	{
		Game::PlaySystemSound("Pause_enter");
		Game::SetBGMVolume(0.2f);
		pauseEntered = true;
	}

	// Play a sound and translate the menu in (?)
	float t = m_transitionTimer.GetElapsedTimeNormalized();
	m_menuStartPosition.y = Interpolate(0, 40, t);

	if (m_transitionTimer.HasIntervalElapsed())
	{
		pauseEntered = false;
		return true;
	}

	return false;
}


//-----------------------------------------------------------------------------------------------
// Update
//
void PlayState_Pause::Update()
{
	if (m_resumePressed)
	{
		m_gameState->PopOverrideState();
	}
}


//-----------------------------------------------------------------------------------------------
// Leave
//
bool PlayState_Pause::Leave()
{
	// More sound and stuff here, volume adjustment
	static bool isParticalized = false;
	if (!isParticalized)
	{
		// Particalize the menu
		VoxelFont* menuFont = Game::GetMenuFont();

		VoxelFontDraw_t options;
		options.mode = VOXEL_FONT_FILL_NONE;
		options.glyphColors.push_back(Rgba::BLUE);
		options.fillColor = Rgba::BLUE;
		options.font = menuFont;
		options.alignment = Vector3(0.5f, 0.5f, 0.5f);
		options.borderThickness = 0;
		options.scale = IntVector3::ONES;
		options.up = IntVector3(0, 0, 1);

		// Draw the menu options
		IntVector3 drawPosition = IntVector3(128, 40, 160);
		drawPosition -= IntVector3(0, 0, 1) * (menuFont->GetGlyphDimensions().y + 5);

		for (int i = 0; i < m_menuText.size(); ++i)
		{
			options.glyphColors[0] = Rgba::BLUE;

			if (i == m_cursorIndex)
			{
				float time = m_transitionTimer.GetElapsedTime();
				float t = 0.5f * (SinDegrees(1000.f * time) + 1.0f);

				options.glyphColors[0] = Interpolate(Rgba::BLUE, Rgba::WHITE, t);
			}

			Game::GetWorld()->ParticalizeVoxelText(m_menuText[i], drawPosition, options);
			drawPosition -= IntVector3(0, 0, 1) * (menuFont->GetGlyphDimensions().y + 5);
			if (i == 1)
			{
				drawPosition -= IntVector3(0, 0, 1) * (menuFont->GetGlyphDimensions().y + 5);
			}
		}

		isParticalized = true;
	}

	UpdateWorldAndCamera();

	bool finishedLeaving = m_transitionTimer.HasIntervalElapsed();

	if (finishedLeaving)
	{
		isParticalized = false;
		Game::PlaySystemSound("Pause_leave");
		Game::SetBGMVolume(1.0f);
	}

	return finishedLeaving;
}


//-----------------------------------------------------------------------------------------------
// Renders the enter transition
//
void PlayState_Pause::Render_Enter() const
{
	Render();
}


//-----------------------------------------------------------------------------------------------
// Renders the running state
//
void PlayState_Pause::Render() const
{
	Game::GetWorld()->DrawToGrid();
	Game::DrawPlayerHUD();

	VoxelFont* menuFont = Game::GetMenuFont();

	VoxelFontDraw_t options;
	options.mode = VOXEL_FONT_FILL_NONE;
	options.glyphColors.push_back(Rgba::BLUE);
	options.fillColor = Rgba::BLUE;
	options.font = menuFont;
	options.alignment = Vector3(0.5f, 0.5f, 0.5f);
	options.borderThickness = 0;
	options.scale = IntVector3::ONES;
	options.up = IntVector3(0, 0, 1);

	// Draw the menu options
	IntVector3 drawPosition = m_menuStartPosition;
	drawPosition -= IntVector3(0, 0, 1) * (menuFont->GetGlyphDimensions().y + 5);

	for (int i = 0; i < m_menuText.size(); ++i)
	{
		options.glyphColors[0] = Rgba::BLUE;

		if (i == m_cursorIndex)
		{
			float time = m_transitionTimer.GetElapsedTime();
			float t = 0.5f * (SinDegrees(1000.f * time) + 1.0f);

			options.glyphColors[0] = Interpolate(Rgba::BLUE, Rgba::WHITE, t);
		}

		Game::GetVoxelGrid()->DrawVoxelText(m_menuText[i], drawPosition, options);
		drawPosition -= IntVector3(0, 0, 1) * (menuFont->GetGlyphDimensions().y + 5);
		if (i == 1)
		{
			drawPosition -= IntVector3(0, 0, 1) * (menuFont->GetGlyphDimensions().y + 5);
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Renders the leave transition
//
void PlayState_Pause::Render_Leave() const
{
	Game::GetWorld()->DrawToGrid();
	Game::DrawPlayerHUD();
}


//-----------------------------------------------------------------------------------------------
// Processes the selection a player made on the menu
//
void PlayState_Pause::ProcessSelection()
{
	if (m_cursorIndex == 2) // Resume
	{
		m_resumePressed = true;
	}
	else if (m_cursorIndex == 3) // Quit
	{
		Game::TransitionToGameState(new GameState_MainMenu());
	}
}
