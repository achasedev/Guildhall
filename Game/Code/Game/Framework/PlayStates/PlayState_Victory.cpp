/************************************************************************/
/* File: PlayState_Victory.cpp
/* Author: Andrew Chase
/* Date: October 25th 2018
/* Description: Implementation of the Victory PlayState
/************************************************************************/
#include "Game/Framework/Game.hpp"
#include "Game/Framework/World.hpp"
#include "Game/Framework/VoxelFont.hpp"
#include "Game/Framework/GameAudioSystem.hpp"
#include "Game/Framework/CampaignManager.hpp"
#include "Game/Framework/CampaignDefinition.hpp"
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
	if (m_state == TRANSITION_STATE_UPDATING)
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
}


//-----------------------------------------------------------------------------------------------
// Enter update
//
bool PlayState_Victory::Enter()
{
	if (m_transitionTimer.HasIntervalElapsed())
	{
		if (Game::GetCampaignManager()->GetCurrentCampaignDefinition()->m_hasLeaderboards)
		{
			Game::UpdateLeaderboardWithCurrentScore();
		}

		Game::GetGameAudioSystem()->PlayBGM("Data/Audio/Music/Victory.wav");
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
// Renders the running state
//
void PlayState_Victory::Render() const
{
	Game::GetWorld()->DrawToGrid();

	// Draw the victory text
	VoxelFont* menuFont = Game::GetMenuFont();

	VoxelFontDraw_t options;
	options.mode = VOXEL_FONT_FILL_NONE;
	options.glyphColors.push_back(Rgba::BLUE);
	options.fillColor = Rgba::BLUE;
	options.font = menuFont;
	options.scale = IntVector3(2, 2, 1);
	options.up = IntVector3(0, 1, 0);
	options.alignment = Vector3(0.5f, 0.5f, 0.5f);
	options.borderThickness = 0;
	options.colorFunction = GetColorForWaveEffect;

	VoxelFontColorWaveArgs_t colorArgs;
	colorArgs.direction = IntVector3(1, 0, 0);
	colorArgs.speed = 1.0f;

	options.colorFunctionArgs = &colorArgs;
	options.offsetFunction = GetOffsetForFontWaveEffect;

	Game::GetVoxelGrid()->DrawVoxelText("Victory", IntVector3(128, 32, 255), options);
	options.offsetFunction = nullptr;
	options.colorFunction = nullptr;

	options.scale = IntVector3::ONES;
	options.up = IntVector3(0, 0, 1);

	// Draw the leaderboard
	IntVector3 drawPosition = IntVector3(128, 40, 160);

	Leaderboard leaderboardToDisplay;
	
	if (Game::GetCampaignManager()->GetCurrentCampaignDefinition()->m_hasLeaderboards)
	{
		leaderboardToDisplay = Game::GetLeaderboardForCurrentCampaign();
	}

	const ScoreBoard& scoreboard = leaderboardToDisplay.m_scoreboards[Game::GetCurrentPlayerCount() - 1];

	Game::GetVoxelGrid()->DrawVoxelText(leaderboardToDisplay.m_name, drawPosition, options);
	drawPosition -= IntVector3(0, 0, 1) * (menuFont->GetGlyphDimensions().y + 5);

	Game::GetVoxelGrid()->DrawVoxelText(scoreboard.m_name, drawPosition, options);
	drawPosition -= IntVector3(0, 0, 1) * (menuFont->GetGlyphDimensions().y + 5);

	bool currentScoreRendered = false;
	Rgba flashColor;
	for (int i = 0; i < NUM_SCORES_PER_SCOREBOARD; ++i)
	{
		if (!currentScoreRendered && scoreboard.m_scores[i] == Game::GetScore())
		{
			float time = m_transitionTimer.GetElapsedTime();
			float t = 0.5f * (SinDegrees(1000.f * time) + 1.0f);

			flashColor = Interpolate(m_leaderboardTextColor, m_scoresFlashColor, t);
			options.glyphColors[0] = flashColor;

			currentScoreRendered = true;
		}
		else
		{
			options.glyphColors[0] = m_leaderboardTextColor;
		}

		Game::GetVoxelGrid()->DrawVoxelText(Stringf("%i", scoreboard.m_scores[i]), drawPosition, options);
		drawPosition -= IntVector3(0, 0, 1) * (menuFont->GetGlyphDimensions().y + 5);
	}

	if (currentScoreRendered)
	{
		drawPosition -= IntVector3(0, 0, 1) * (menuFont->GetGlyphDimensions().y + 5);
		options.glyphColors[0] = flashColor;
		Game::GetVoxelGrid()->DrawVoxelText("New Record", drawPosition, options);
		drawPosition -= IntVector3(0, 0, 1) * (menuFont->GetGlyphDimensions().y + 5);
	}

	drawPosition -= IntVector3(0, 0, 10);

	options.glyphColors[0] = m_leaderboardTextColor;
	options.offsetFunction = GetOffsetForFontWaveEffect;
	options.colorFunction = GetColorForWaveEffect;

	Game::GetVoxelGrid()->DrawVoxelText("Press A", drawPosition, options);
	drawPosition -= IntVector3(0, 0, 1) * (menuFont->GetGlyphDimensions().y + 5);

	Game::GetVoxelGrid()->DrawVoxelText("to return", drawPosition, options);
}


//-----------------------------------------------------------------------------------------------
// Renders the leave transition
//
void PlayState_Victory::Render_Leave() const
{
	Game::GetWorld()->DrawToGrid();
	Game::DrawPlayerHUD();
}
