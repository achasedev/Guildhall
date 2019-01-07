/************************************************************************/
/* File: PlayState_Defeat.cpp
/* Author: Andrew Chase
/* Date: October 24th 2018
/* Description: Implementation of the Defeat PlayState class
/************************************************************************/
#include "Game/Framework/Game.hpp"
#include "Game/Framework/World.hpp"
#include "Game/Framework/VoxelFont.hpp"
#include "Game/GameStates/GameState_MainMenu.hpp"
#include "Game/Framework/PlayStates/PlayState_Defeat.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Input/InputSystem.hpp"


//-----------------------------------------------------------------------------------------------
// Constructor
//
PlayState_Defeat::PlayState_Defeat()
	: PlayState(DEFEAT_TRANSITION_IN_TIME, DEFEAT_TRANSITION_OUT_TIME)
{
}


//-----------------------------------------------------------------------------------------------
// Destructor
//
PlayState_Defeat::~PlayState_Defeat()
{
}


//-----------------------------------------------------------------------------------------------
// Checks for input to return to the main menu
//
void PlayState_Defeat::ProcessInput()
{
	InputSystem* input = InputSystem::GetInstance();
	bool spacePressed = input->WasKeyJustPressed(' ');

	if (spacePressed)
	{
		Game::GetInstance()->TransitionToGameState(new GameState_MainMenu());
	}
	else
	{
		// Check controllers
		for (int i = 0; i < MAX_PLAYERS; ++i)
		{
			XboxController& controller = input->GetController(i);

			if (controller.WasButtonJustPressed(XBOX_BUTTON_START) || controller.WasButtonJustPressed(XBOX_BUTTON_A))
			{
				Game::GetInstance()->TransitionToGameState(new GameState_MainMenu());
			}
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Enter - does nothing for now
//
bool PlayState_Defeat::Enter()
{
	if (m_transitionTimer.HasIntervalElapsed())
	{
		Game::UpdateLeaderboardWithCurrentScore();
		return true;
	}

	return false;
}


//-----------------------------------------------------------------------------------------------
// Update - doesn't update anything
//
void PlayState_Defeat::Update()
{
	// Nothing!
}


//-----------------------------------------------------------------------------------------------
// Leave - does nothing for now
//
bool PlayState_Defeat::Leave()
{
	return m_transitionTimer.HasIntervalElapsed();
}


//-----------------------------------------------------------------------------------------------
// Renders the enter transition of the defeat state
//
void PlayState_Defeat::Render_Enter() const
{
	Game::GetWorld()->DrawToGrid();
	Game::DrawPlayerHUD();
}


//-----------------------------------------------------------------------------------------------
// Renders the defeat state screen and text
//
void PlayState_Defeat::Render() const
{
	Game::GetWorld()->DrawToGrid();

	// Draw the victory text
	VoxelFont* menuFont = Game::GetMenuFont();

	VoxelFontDraw_t options;
	options.mode = VOXEL_FONT_FILL_NONE;
	options.textColor = Rgba::BLUE;
	options.fillColor = Rgba::BLUE;
	options.font = menuFont;
	options.scale = IntVector3(2, 2, 1);
	options.up = IntVector3(0, 1, 0);
	options.alignment = Vector3(0.5f, 0.5f, 0.5f);
	options.borderThickness = 0;

	Game::GetVoxelGrid()->DrawVoxelText("Defeat", IntVector3(128, 32, 255), options, GetOffsetForFontWaveEffect);

	options.scale = IntVector3::ONES;
	options.up = IntVector3(0, 0, 1);

	// Draw the leaderboard
	IntVector3 drawPosition = IntVector3(128, 40, 160);

	const Leaderboard& board = Game::GetLeaderboards()[Game::GetCurrentPlayerCount() - 1];

	Game::GetVoxelGrid()->DrawVoxelText(board.m_name, drawPosition, options);
	drawPosition -= IntVector3(0, 0, 1) * (menuFont->GetGlyphDimensions().y + 5);

	bool currentScoreRendered = false;
	for (int i = 0; i < NUM_SCORES_PER_LEADERBOARD; ++i)
	{
		if (!currentScoreRendered && board.m_scores[i] == Game::GetScore())
		{
			float time = m_transitionTimer.GetElapsedTime();
			float t = 0.5f * (SinDegrees(1000.f * time) + 1.0f);

			options.textColor = Interpolate(m_leaderboardTextColor, m_scoresFlashColor, t);

			currentScoreRendered = true;
		}
		else
		{
			options.textColor = m_leaderboardTextColor;
		}

		Game::GetVoxelGrid()->DrawVoxelText(Stringf("%i", board.m_scores[i]), drawPosition, options);
		drawPosition -= IntVector3(0, 0, 1) * (menuFont->GetGlyphDimensions().y + 5);
	}

	drawPosition -= IntVector3(0, 0, 10);

	Game::GetVoxelGrid()->DrawVoxelText("Press A", drawPosition, options, GetOffsetForFontWaveEffect);
	drawPosition -= IntVector3(0, 0, 1) * (menuFont->GetGlyphDimensions().y + 5);

	Game::GetVoxelGrid()->DrawVoxelText("to return", drawPosition, options, GetOffsetForFontWaveEffect);
}


//-----------------------------------------------------------------------------------------------
// Renders the leave transition of the defeat state
//
void PlayState_Defeat::Render_Leave() const
{
	Game::GetWorld()->DrawToGrid();
	Game::DrawPlayerHUD();
}
