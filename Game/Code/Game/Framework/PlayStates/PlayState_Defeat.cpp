/************************************************************************/
/* File: PlayState_Defeat.cpp
/* Author: Andrew Chase
/* Date: October 24th 2018
/* Description: Implementation of the Defeat PlayState class
/************************************************************************/
#include "Game/Framework/Game.hpp"
#include "Game/Framework/World.hpp"
#include "Game/Framework/VoxelFont.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Game/GameStates/GameState_MainMenu.hpp"
#include "Game/Framework/PlayStates/PlayState_Defeat.hpp"

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
	Game::DrawPlayerHUD();

	// Draw the victory text
	VoxelFont* menuFont = Game::GetMenuFont();

	VoxelFontDraw_t options;
	options.mode = VOXEL_FONT_FILL_NONE;
	options.textColor = Rgba::BLUE;
	options.fillColor = Rgba::BLUE;
	options.font = menuFont;
	options.scale = IntVector3(1, 1, 1);
	options.up = IntVector3(0, 1, 0);
	options.alignment = Vector3(0.5f, 0.5f, 0.5f);
	options.borderThickness = 0;

	Game::GetVoxelGrid()->DrawVoxelText("Victory", IntVector3(128, 8, 255), options);

	// Draw the leaderboard
	IntVector3 drawPosition = IntVector3(128, 50, 160);
	options.up = IntVector3(0, 0, 1);

	const Leaderboard& board = Game::GetLeaderboards()[Game::GetCurrentPlayerCount()];

	Game::GetVoxelGrid()->DrawVoxelText(board.m_name, drawPosition, options);
	drawPosition -= IntVector3(0, 0, 1) * (menuFont->GetGlyphDimensions().y + 5);

	for (int i = 0; i < NUM_SCORES_PER_LEADERBOARD; ++i)
	{
		Game::GetVoxelGrid()->DrawVoxelText(Stringf("%i", board.m_scores[i]), drawPosition, options);
		drawPosition -= IntVector3(0, 0, 1) * (menuFont->GetGlyphDimensions().y + 5);
	}

	Game::GetVoxelGrid()->DrawVoxelText("Press A", drawPosition, options);
}


//-----------------------------------------------------------------------------------------------
// Renders the leave transition of the defeat state
//
void PlayState_Defeat::Render_Leave() const
{
	Game::GetWorld()->DrawToGrid();
	Game::DrawPlayerHUD();
}
