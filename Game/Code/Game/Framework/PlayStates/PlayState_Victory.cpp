/************************************************************************/
/* File: PlayState_Victory.cpp
/* Author: Andrew Chase
/* Date: October 25th 2018
/* Description: Implementation of the Victory PlayState
/************************************************************************/
#include "Game/Framework/Game.hpp"
#include "Game/Framework/World.hpp"
#include "Game/Framework/VoxelFont.hpp"
#include "Game/GameStates/GameState_MainMenu.hpp"
#include "Game/Framework/PlayStates/PlayState_Victory.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Time/Clock.hpp"
#include "Engine/Input/InputSystem.hpp"

//-----------------------------------------------------------------------------------------------
// Constructor
//
PlayState_Victory::PlayState_Victory()
	: PlayState(VICTORY_TRANSITION_IN_TIME, VICTORY_TRANSITION_OUT_TIME)
{
}


//-----------------------------------------------------------------------------------------------
// Destructor
//
PlayState_Victory::~PlayState_Victory()
{
}


//-----------------------------------------------------------------------------------------------
// Checks for input to return to the main menu
//
void PlayState_Victory::ProcessInput()
{
	InputSystem* input = InputSystem::GetInstance();

	// Keyboard input
	bool spacePressed = input->WasKeyJustPressed(' ');

	if (spacePressed)
	{
		Game::GetInstance()->TransitionToGameState(new GameState_MainMenu());
	}
	else
	{
		// Check all controllers for press
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
// Enter update
//
bool PlayState_Victory::Enter()
{
	if (m_transitionTimer.HasIntervalElapsed())
	{
		Game::UpdateLeaderboardWithCurrentScore();
		Game::PlayBGM("Data/Music/Victory.wav");
	}

	return m_transitionTimer.HasIntervalElapsed();
}


//-----------------------------------------------------------------------------------------------
// Update
//
void PlayState_Victory::Update()
{
	// Nothing!
}


//-----------------------------------------------------------------------------------------------
// Leave
//
bool PlayState_Victory::Leave()
{
	return m_transitionTimer.HasIntervalElapsed();
}


//-----------------------------------------------------------------------------------------------
// Renders the enter transition
//
void PlayState_Victory::Render_Enter() const
{
	Game::GetWorld()->DrawToGrid();
	Game::DrawPlayerHUD();
}


//-----------------------------------------------------------------------------------------------
// Returns the color that should be used to render the current score on the leaderboard, for
// flashing effect
//
Rgba PlayState_Victory::GetColorForCurrentScore() const
{
	float time = m_transitionTimer.GetElapsedTime();
	float t = 0.5f * (SinDegrees(1000.f * time) + 1.0f);

	return Interpolate(m_leaderboardTextColor, m_scoresFlashColor, t);
}


//-----------------------------------------------------------------------------------------------
// Returns an offset to be applied to the current voxel at local coords when rendering fonts
//
IntVector3 GetOffsetForFontWaveEffect(const IntVector3& textDimensions, const IntVector3& localCoords)
{
	int frontRange = 15;
	int rearRange = 50;
	float maxOffset = 10;

	int time = (int) (100.f * Game::GetGameClock()->GetTotalSeconds());

	int target = (time % (2 * textDimensions.x)) - frontRange;
	int displacement = localCoords.x - target;
	int distance = AbsoluteValue(displacement);

	IntVector3 offset = IntVector3::ZERO;

	if (displacement >= 0 && displacement <= frontRange)
	{
		float t = (float)(frontRange - distance) / (float)frontRange;
		offset.z = (int)(-maxOffset * t);
	}
	else if (displacement < 0 && displacement > -rearRange)
	{
		float t = (float)(rearRange - distance) / (float)(rearRange);
		offset.z = (int)(-maxOffset * t);
	}

	return offset;
}


//-----------------------------------------------------------------------------------------------
// Renders the running state
//
void PlayState_Victory::Render() const
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

	Game::GetVoxelGrid()->DrawVoxelText("Victory", IntVector3(128, 32, 255), options, GetOffsetForFontWaveEffect);

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
			options.textColor = GetColorForCurrentScore();
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
// Renders the leave transition
//
void PlayState_Victory::Render_Leave() const
{
	Game::GetWorld()->DrawToGrid();
	Game::DrawPlayerHUD();
}
