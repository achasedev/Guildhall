/************************************************************************/
/* File: Game.cpp
/* Author: Andrew Chase
/* Date: March 24th, 2018
/* Description: Game class for general gameplay management
/************************************************************************/
#include "Game/Entity/Weapon.hpp"
#include "Game/Entity/Player.hpp"
#include "Game/Framework/Game.hpp"
#include "Game/Framework/World.hpp"
#include "Game/Framework/VoxelGrid.hpp"
#include "Game/Framework/VoxelFont.hpp"
#include "Game/Framework/GameCommon.hpp"
#include "Game/Framework/GameCamera.hpp"
#include "Game/Framework/CampaignManager.hpp"
#include "Game/GameStates/GameState_Loading.hpp"

#include "Engine/Core/File.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/LogSystem.hpp"
#include "Engine/Core/Time/Clock.hpp"
#include "Engine/Rendering/Core/Camera.hpp"
#include "Engine/Rendering/Core/Renderer.hpp"
#include "Engine/Core/Utility/XmlUtilities.hpp"
#include "Engine/Rendering/Core/RenderScene.hpp"
#include "Engine/Core/DeveloperConsole/Command.hpp"
#include "Engine/Rendering/DebugRendering/DebugRenderSystem.hpp"

void Command_KillAll(Command& cmd)
{
	UNUSED(cmd);

	Game::GetWorld()->ParticalizeAllEntities();
}

// The singleton instance
Game* Game::s_instance = nullptr;

//-----------------------------------------------------------------------------------------------
// Default constructor, initialize any game members here (private)
//
Game::Game()
	: m_currentState(new GameState_Loading())
{
	// Clock
	m_gameClock = new Clock(Clock::GetMasterClock());
	m_gameClock->SetMaxDeltaTime((1.f / 30.f)); // Limit the time step to be no more than 1/30th a frame

	// Camera
	Renderer* renderer = Renderer::GetInstance();
	m_gameCamera = new GameCamera();
	m_gameCamera->SetColorTarget(renderer->GetDefaultColorTarget());
	m_gameCamera->SetDepthTarget(renderer->GetDefaultDepthTarget());
	m_gameCamera->SetProjectionPerspective(45.f, 0.1f, 1000.f);
	m_gameCamera->LookAt(Vector3(128.f, 185.f, -90.0f), Vector3(0.f, 200.f, 0.f));

	DebugRenderSystem::SetWorldCamera(m_gameCamera);

	// Game world
	m_world = new World();

	// Voxel Grid
	m_voxelGrid = new VoxelGrid();
	m_voxelGrid->Initialize(IntVector3(256, 64, 256));

	// VoxelFont
	m_hudFont = new VoxelFont("HUD", "Data/Images/Fonts/VoxelFont.png");
	m_menuFont = new VoxelFont("Menu", "Data/Images/Fonts/VoxelFont.png");

	for (int i = 0; i < MAX_PLAYERS; ++i)
	{
		m_players[i] = nullptr;
	}
}


//-----------------------------------------------------------------------------------------------
// Checks if there's currently a crossfade in the music, and adjust volumes accordingly
//
void Game::UpdateMusicCrossfade()
{
	// Check volume
	if (m_trackTendingToTarget != MISSING_SOUND_ID && m_tendingTargetCurrentVolume != m_targetMusicVolume)
	{
		float delta = m_gameClock->GetDeltaTime();

		if (m_tendingTargetCurrentVolume < m_targetMusicVolume)
		{
			m_tendingTargetCurrentVolume = ClampFloat(m_tendingTargetCurrentVolume + delta, 0.f, s_instance->m_targetMusicVolume);
		}
		else
		{
			m_tendingTargetCurrentVolume = ClampFloat(m_tendingTargetCurrentVolume - delta, s_instance->m_targetMusicVolume, 1.0f);
		}

		AudioSystem* audio = AudioSystem::GetInstance();
		audio->SetSoundPlaybackVolume(m_trackTendingToTarget, m_tendingTargetCurrentVolume);
	}

	if (m_trackTendingToZero != MISSING_SOUND_ID)
	{
		float delta = s_instance->m_gameClock->GetDeltaTime();

		s_instance->m_tendingZeroCurrentVolume = ClampFloatZeroToOne(s_instance->m_tendingZeroCurrentVolume - delta);

		AudioSystem* audio = AudioSystem::GetInstance();

		if (m_tendingZeroCurrentVolume == 0.f)
		{
			audio->StopSound(m_trackTendingToZero);
			m_trackTendingToZero = MISSING_SOUND_ID;
		}
		else
		{
			audio->SetSoundPlaybackVolume(m_trackTendingToZero, m_tendingZeroCurrentVolume);
		}
	}

// 	if (s_instance->m_musicCrossfading)
// 	{
// 		float t = s_instance->m_targetMusicVolume * ClampFloatZeroToOne(s_instance->m_musicCrossfadeTimer.GetElapsedTimeNormalized());
// 
// 		AudioSystem* audio = AudioSystem::GetInstance();
// 
// 		if (s_instance->m_trackTendingToZero != MISSING_SOUND_ID)
// 		{
// 			audio->SetSoundPlaybackVolume(m_trackTendingToZero, t);
// 		}
// 
// 		if (s_instance->m_trackTendingToTarget != MISSING_SOUND_ID)
// 		{
// 			audio->SetSoundPlaybackVolume(m_trackTendingToTarget, s_instance->m_targetMusicVolume - t);
// 		}
// 
// 		if (t == s_instance->m_targetMusicVolume)
// 		{
// 			if (s_instance->m_trackTendingToTarget != MISSING_SOUND_ID)
// 			{
// 				audio->StopSound(m_trackTendingToTarget);
// 			}
// 
// 			s_instance->m_trackTendingToTarget = s_instance->m_trackTendingToZero;
// 			s_instance->m_trackTendingToZero = -1;
// 
// 			s_instance->m_musicCrossfadeTimer.Reset();
// 			s_instance->m_musicCrossfading = false;
// 		}
// 	}
// 
// 	// Check volume
// 	if (s_instance->m_tendingTargetCurrentVolume != s_instance->m_targetMusicVolume)
// 	{
// 		float delta = s_instance->m_gameClock->GetDeltaTime();
// 
// 		if (s_instance->m_tendingTargetCurrentVolume < s_instance->m_targetMusicVolume)
// 		{
// 			s_instance->m_tendingTargetCurrentVolume = ClampFloat(s_instance->m_tendingTargetCurrentVolume + delta, 0.f, s_instance->m_targetMusicVolume);
// 		}
// 		else
// 		{
// 			s_instance->m_tendingTargetCurrentVolume = ClampFloat(s_instance->m_tendingTargetCurrentVolume - delta, s_instance->m_targetMusicVolume, 1.0f);
// 		}
// 
// 		AudioSystem* audio = AudioSystem::GetInstance();
// 		audio->SetSoundPlaybackVolume(s_instance->m_trackTendingToTarget, s_instance->m_tendingTargetCurrentVolume);
// 	}
}


//-----------------------------------------------------------------------------------------------
// Increments the score displayed to count up to the actual score, for cool effect
//
void Game::UpdateDisplayedScore()
{
	if (AreMostlyEqual(m_actualScore, m_displayedScore, 0.0001f))
	{
		return;
	}

	float thresholdBeforeSlowingDown = 500.f;
	float minRate = 300.f;
	float maxRate = 1500.f;
	float rate = 0.f;

	float diff = AbsoluteValue(m_actualScore - m_displayedScore);
	if (diff > thresholdBeforeSlowingDown)
	{
		rate = maxRate;
	}
	else
	{
		float rateScalar = RangeMapFloat(diff, 0.f, 1000.f, 0.f, 1.f);
		rate = maxRate * rateScalar;

		rate = ClampFloat(rate, minRate, maxRate);
	}

	if (m_actualScore < m_displayedScore)
	{
		rate *= -1.f;
	}

	float deltaScore = GetDeltaTime() * rate;

	if (m_actualScore > m_displayedScore)
	{
		m_displayedScore = ClampFloat(m_displayedScore + deltaScore, 0.f, m_actualScore);
	}
	else
	{
		m_displayedScore = ClampFloat(m_displayedScore + deltaScore, m_actualScore, 9999999.f);
	}
}


//-----------------------------------------------------------------------------------------------
// Loads the leaderboards from file, if one exists
//
void Game::LoadLeaderboardsFromFile()
{
	File file;
	bool success = file.Open("Data/Leaderboards.txt", "r");

	if (!success)
	{
		// Initialize all current leaderboards to 0
		for (int leaderboardIndex = 0; leaderboardIndex < NUM_LEADERBOARDS; ++leaderboardIndex)
		{
			Leaderboard& board = m_leaderboards[leaderboardIndex];

			if (leaderboardIndex == 0)
			{
				board.m_name = Stringf("%i Player", leaderboardIndex + 1);

			}
			else
			{
				board.m_name = Stringf("%i Players", leaderboardIndex + 1);
			}

			for (int scoreIndex = 0; scoreIndex < NUM_SCORES_PER_LEADERBOARD; ++scoreIndex)
			{
				board.m_scores[scoreIndex] = 0;
			}
		}
	}
	else
	{
		file.LoadFileToMemory();

		// Read off each leaderboard
		for (int leaderboardIndex = 0; leaderboardIndex < NUM_LEADERBOARDS; ++leaderboardIndex)
		{
			Leaderboard& board = m_leaderboards[leaderboardIndex];

			file.GetNextLine(board.m_name);

			for (int scoreIndex = 0; scoreIndex < NUM_SCORES_PER_LEADERBOARD; ++scoreIndex)
			{
				std::string scoreText;
				file.GetNextLine(scoreText);
				int score = StringToInt(scoreText);

				board.m_scores[scoreIndex] = score;
			}
		}

		file.Close();
	}
}


//-----------------------------------------------------------------------------------------------
// Writes the leaderboards to a text file
//
void Game::WriteLeaderboardsToFile()
{
	File file;

	bool success = file.Open("Data/Leaderboards.txt", "w");

	
	if (!success)
	{
		LogTaggedPrintf("GAME", "Error: Couldn't open the leaderboard file for write");
	}
	else
	{
		for (int leaderboardIndex = 0; leaderboardIndex < NUM_LEADERBOARDS; ++leaderboardIndex)
		{
			Leaderboard& board = m_leaderboards[leaderboardIndex];

			file.Write(board.m_name.c_str(), board.m_name.size());
			file.Write("\n", 1);

			for (int scoreIndex = 0; scoreIndex < NUM_SCORES_PER_LEADERBOARD; ++scoreIndex)
			{
				std::string scoreText = Stringf("%i\n", board.m_scores[scoreIndex]);
				file.Write(scoreText.c_str(), scoreText.size());
			}
		}

		file.Close();
	}
}


//-----------------------------------------------------------------------------------------------
// Destructor - clean up any allocated members (private)
//
Game::~Game()
{
	if (m_world != nullptr)
	{
		delete m_world;
		m_world = nullptr;
	}

	if (m_voxelGrid != nullptr)
	{
		delete m_voxelGrid;
		m_voxelGrid = nullptr;
	}

	delete m_hudFont;
	m_hudFont = nullptr;

	delete m_menuFont;
	m_menuFont = nullptr;
}


//-----------------------------------------------------------------------------------------------
// Constructs the singleton game instance
//
void Game::Initialize()
{
	GUARANTEE_OR_DIE(s_instance == nullptr, "Error: Game::Initialize called when a Game instance already exists.");
	s_instance = new Game();

	// Campaign manager initialize here, since it refers to the instance above
	s_instance->m_campaignManager = new CampaignManager();

	// Set the game clock on the Renderer
	Renderer::GetInstance()->SetRendererGameClock(s_instance->m_gameClock);

	// Set up the mouse for FPS controls (Free camera)
	Mouse& mouse = InputSystem::GetMouse();

	mouse.ShowMouseCursor(false);
	mouse.LockCursorToClient(true);
	mouse.SetCursorMode(CURSORMODE_RELATIVE);

	// Commands
	Command::Register("killall", "Kills all entities", Command_KillAll);

	s_instance->LoadLeaderboardsFromFile();
}


//-----------------------------------------------------------------------------------------------
// Deletes the singleton instance
//
void Game::ShutDown()
{
	s_instance->WriteLeaderboardsToFile();

	delete s_instance;
	s_instance = nullptr;
}


//-----------------------------------------------------------------------------------------------
// Checks for input received this frame and updates states accordingly
//
void Game::ProcessInput()
{
	if (m_currentState != nullptr)
	{
		m_currentState->ProcessInput();
	}

	m_gameCamera->ProcessInput();
}


//-----------------------------------------------------------------------------------------------
// Update the movement variables of every entity in the world, as well as update the game state
// based on the input this frame
//
void Game::Update()
{
	if (m_gameStateState == GAME_STATE_TRANSITIONING_OUT)
	{
		// Update on leave of the current state
		if (m_currentState != nullptr)
		{
			bool leaveFinished = m_currentState->Leave();

			if (leaveFinished)
			{
				delete m_currentState;
				m_currentState = m_transitionState;
				m_transitionState = nullptr;

				m_currentState->StartEnterTimer();
				m_gameStateState = GAME_STATE_TRANSITIONING_IN;
			}
		}
	}
	
	if (m_gameStateState == GAME_STATE_TRANSITIONING_IN) // Update on enter of the transition state
	{
		bool enterFinished = m_currentState->Enter();

		if (enterFinished)
		{
			m_gameStateState = GAME_STATE_UPDATING;
		}
	}
	
	if (m_gameStateState == GAME_STATE_UPDATING)
	{
		m_currentState->Update();
	}

	m_gameCamera->UpdateBasedOnState();

	// Update any music crossfades
	UpdateMusicCrossfade();
	UpdateDisplayedScore();
}

#include "Engine/Assets/AssetDB.hpp"
//-----------------------------------------------------------------------------------------------
// Draws to screen
//
void Game::Render() const
{
	PROFILE_LOG_SCOPE("First-Stage Render (Look at self time)");

	m_voxelGrid->Clear();

	switch (m_gameStateState)
	{
	case GAME_STATE_TRANSITIONING_IN:
		m_currentState->Render_Enter();
		break;
	case GAME_STATE_UPDATING:
		m_currentState->Render();
		break;
	case GAME_STATE_TRANSITIONING_OUT:
		m_currentState->Render_Leave();
		break;
	default:
		break;
	}

	m_voxelGrid->BuildMeshAndDraw();

	Renderer* renderer = Renderer::GetInstance();
	renderer->SetCurrentCamera(renderer->GetUICamera());
	AABB2 uibounds = renderer->GetUIBounds();

	renderer->DrawTextInBox2D("Alpha v0.8", uibounds, Vector2::ZERO, 20.f, TEXT_DRAW_OVERRUN, AssetDB::GetBitmapFont("Data/Images/Fonts/ConsoleFont.png"), Rgba::GREEN);
}


//-----------------------------------------------------------------------------------------------
// Returns the game state of the Game instance
//
GameState* Game::GetGameState() const
{
	return m_currentState;
}


//-----------------------------------------------------------------------------------------------
// Sets the pending state flag to the one given, so the next frame the game will switch to the
// given state
//
void Game::TransitionToGameState(GameState* newState)
{
	s_instance->m_transitionState = newState;
	s_instance->m_gameStateState = GAME_STATE_TRANSITIONING_OUT;

	if (s_instance->m_currentState != nullptr)
	{
		s_instance->m_currentState->StartLeaveTimer();
	}
}


//-----------------------------------------------------------------------------------------------
// Returns the VoxelFont used for the menu screens
//
VoxelFont* Game::GetMenuFont()
{
	return s_instance->m_menuFont;
}


//-----------------------------------------------------------------------------------------------
// Returns the voxel grid used by the game
//
VoxelGrid* Game::GetVoxelGrid()
{
	return s_instance->m_voxelGrid;
}


//-----------------------------------------------------------------------------------------------
// Returns the game clock
//
Clock* Game::GetGameClock()
{
	return s_instance->m_gameClock;
}


//-----------------------------------------------------------------------------------------------
// Returns the camera used to render game elements
//
GameCamera* Game::GetGameCamera()
{
	return s_instance->m_gameCamera;
}


//-----------------------------------------------------------------------------------------------
// Returns the frame time for the game clock
//
float Game::GetDeltaTime()
{
	return s_instance->m_gameClock->GetDeltaTime();
}


//-----------------------------------------------------------------------------------------------
// Returns the world used for all entities and map
//
World* Game::GetWorld()
{
	return s_instance->m_world;
}


//-----------------------------------------------------------------------------------------------
// Sets the current world to the one specified
//
void Game::SetWorld(World* world)
{
	if (s_instance->m_world != nullptr)
	{
		delete s_instance->m_world;
	}

	s_instance->m_world = world;
}


//-----------------------------------------------------------------------------------------------
// Returns the players array (not necessarily full)
//
Player** Game::GetPlayers()
{
	return &s_instance->m_players[0];
}


//-----------------------------------------------------------------------------------------------
// Returns the number of players in the game currently, regardless of whether they're alive
//
int Game::GetCurrentPlayerCount()
{
	int count = 0;
	for (int i = 0; i < MAX_PLAYERS; ++i)
	{
		if (s_instance->m_players[i] != nullptr)
		{
			count++;
		}
	}

	return count;
}


//-----------------------------------------------------------------------------------------------
// Returns the leaderboards for the game
//
const Leaderboard* Game::GetLeaderboards()
{
	return s_instance->m_leaderboards;
}


//-----------------------------------------------------------------------------------------------
// Returns the Campaign Manager for the game
//
CampaignManager* Game::GetCampaignManager()
{
	return s_instance->m_campaignManager;
}


//-----------------------------------------------------------------------------------------------
// Resets the score back to 0
//
void Game::ResetScore()
{
	s_instance->m_actualScore = 0.f;
	s_instance->m_displayedScore = 0.f;
}


//-----------------------------------------------------------------------------------------------
// Adds the given points to the score
//
void Game::AddPointsToScore(int pointsToAdd)
{
	s_instance->m_actualScore = ClampFloat(s_instance->m_actualScore + (float) pointsToAdd, 0.f, 9999999.f);
}


//-----------------------------------------------------------------------------------------------
// Updates the appropriate leaderboard with the given score if the given score makes it
//
void Game::UpdateLeaderboardWithCurrentScore()
{
	// Get the leaderboard
	Leaderboard* board = &s_instance->m_leaderboards[GetCurrentPlayerCount() - 1];

	for (int scoreIndex = 0; scoreIndex < NUM_SCORES_PER_LEADERBOARD; ++scoreIndex)
	{
		if (board->m_scores[scoreIndex] <= s_instance->m_actualScore)
		{
			// Add the score in, pushing all lower scores down
			for (int i = scoreIndex + 1; i < NUM_SCORES_PER_LEADERBOARD; ++i)
			{
				board->m_scores[i] = board->m_scores[i - 1];
			}

			board->m_scores[scoreIndex] = RoundToNearestInt(s_instance->m_actualScore);
			break;
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Returns the current score of the game
//
int Game::GetScore()
{
	return RoundToNearestInt(s_instance->m_actualScore);
}


//- C FUNCTION ----------------------------------------------------------------------------------
// Returns the alignment used to render the given player's HUD
//
Vector3 GetHUDAlignmentForPlayerID(int id)
{
	Vector3 alignment = Vector3::ZERO;

	// HUD is on the right
	if (id % 2 == 1)
	{
		alignment.x = 1.0f;
	}

	return alignment;
}


//- C FUNCTION ----------------------------------------------------------------------------------
// Returns the start coords for the given player's HUD in world coordinates
//
IntVector3 GetHUDStartForPlayerID(int id)
{
	IntVector3 hudStart = IntVector3(2, 34, 252);

	// HUD is on the top (players 1 and 2)
	if (id <= 1)
	{
		hudStart.y += 11;
	}

	// HUD is on the right (players 2 and 4)
	if (id % 2 == 1)
	{
		hudStart.x = 253;
	}

	return hudStart;
}


//- C FUNCTION ----------------------------------------------------------------------------------
// Returns the on-screen text for the player with the given id
//
std::string GetHUDTextForPlayerID(int id)
{
	switch (id)
	{
	case 0:
		return "P1";
		break;
	case 1:
		return "P2";
		break;
	case 2:
		return "P3";
		break;
	case 3:
		return "P4";
		break;
	default:
		return "ERROR";
		break;
	}
}


//- C FUNCTION ----------------------------------------------------------------------------------
// Draws the given player's state to the grid
//
void DrawHUDForPlayer(int playerID, Player* player, VoxelGrid* grid, VoxelFont* font)
{
	VoxelFontDraw_t options;
	options.mode = VOXEL_FONT_FILL_NONE;
	options.fillColor = Player::GetColorForPlayerID(playerID);
	options.font = font;
	options.scale = IntVector3(1, 1, 4);
	options.borderThickness = 0;
	options.alignment = GetHUDAlignmentForPlayerID(playerID);

	IntVector3 startCoords = GetHUDStartForPlayerID(playerID);

	// Assemble the string
	std::string playerHudText;

	// Draw health if they're alive
	if (player != nullptr)
	{
		// Draw the player's health bar
		if (!player->IsRespawning())
		{
			char healthbox = 0;

			for (int i = 0; i < player->GetHealth(); ++i)
			{
				playerHudText += healthbox;
				options.glyphColors.push_back(Rgba::RED);
			}

			// Push missing heath as negative boxes
			for (int i = player->GetHealth(); i < player->GetEntityDefinition()->m_initialHealth; ++i)
			{
				playerHudText += healthbox;
				options.glyphColors.push_back(Rgba(0, 0, 0, 0));
			}
		}
		else
		{
			// Draw "Wait X" if respawning
			playerHudText = Stringf("Wait %i", (int)player->GetRespawnTimeRemaining());

			for (int i = 0; i < (int) playerHudText.size(); ++i)
			{
				options.glyphColors.push_back(Rgba::WHITE);
			}
		}
	}
	else if (player == nullptr && InputSystem::GetInstance()->GetController(playerID).IsConnected())
	{
		// Draw "Press Start" if controller connected but not spawned
		playerHudText = "Press Start";

		for (int i = 0; i < (int)playerHudText.size(); ++i)
		{
			options.glyphColors.push_back(Rgba::WHITE);
		}
	}
	else
	{
		// Draw "No Pad" if no controller connected
		playerHudText = "Connect Ctrl";

		for (int i = 0; i < (int)playerHudText.size(); ++i)
		{
			options.glyphColors.push_back(Rgba::WHITE);
		}
	}


	// Lastly, append/prepend the player icon based on alignment
	if (options.alignment.x == 1.0f)
	{
		std::string iconText = Stringf(" P%i", playerID + 1);
		playerHudText += iconText;

		for (int i = 0; i < (int)iconText.size(); ++i)
		{
			options.glyphColors.push_back(Rgba::WHITE);
		}
	}
	else
	{
		std::string iconText = Stringf("P%i ", playerID + 1);
		playerHudText.insert(0, iconText);

		for (int i = 0; i < (int)iconText.size(); ++i)
		{
			options.glyphColors.insert(options.glyphColors.begin(), Rgba::WHITE);
		}
	}

	// Draw a box background
	IntVector3 boxDimensions = options.font->GetTextDimensions(playerHudText);

	// Add padding for a border
	boxDimensions += IntVector3(3, 3, 0);

	IntVector3 boxStart = startCoords + IntVector3(-2, -2, 2);

	// If the box is on the right, we need to move the box start over to the left
	if (options.alignment.x == 1.0f)
	{
		boxStart.x -= options.font->GetTextDimensions(playerHudText).x;
	}

	// Draw the box first so we can "remove" from it with the empty health boxes
	grid->DrawSolidBox(boxStart, boxDimensions, options.fillColor);
	grid->DrawVoxelText(playerHudText, startCoords, options);
}


//-----------------------------------------------------------------------------------------------
// Draws the in-game HUD that represents current player state
// Multiple game states render this, hence it being placed on Game
//
void Game::DrawPlayerHUD()
{
	VoxelGrid* grid = s_instance->m_voxelGrid;
	Player** players = s_instance->m_players;

	// Draw the player's health/shield bars and weapon indicators
	for (int i = 0; i < MAX_PLAYERS; ++i)
	{
		DrawHUDForPlayer(i, players[i], grid, s_instance->m_hudFont);
	}
}


//-----------------------------------------------------------------------------------------------
// Draw the score at the top of the grid
//
void Game::DrawScore()
{
	DrawHeading(Stringf("Score: %i", RoundToNearestInt(s_instance->m_displayedScore)), IntVector3(128, 56, 252), Vector3(0.5f, 0.f, 0.f));
}


//-----------------------------------------------------------------------------------------------
// Draws the text showing the number of enemies left in the stage
//
void Game::DrawEnemyCountRemaining()
{
	DrawHeading(Stringf("Left: %i", s_instance->m_campaignManager->GetEnemyCountLeftInStage()), IntVector3(0, 56, 252), Vector3::ZERO);
}


//-----------------------------------------------------------------------------------------------
// Draws the text for the stage number and max stage on the UI
//
void Game::DrawStageNumber()
{
	int currStage = s_instance->m_campaignManager->GetCurrentStageNumber(); // Don't +1 here since 0th stage is character select
	DrawHeading(Stringf("Stage: %i", currStage), IntVector3(255, 56, 252), Vector3(1.0f, 0.f, 0.f));
}


//-----------------------------------------------------------------------------------------------
// Draws a heading at the top of the screen
// Used for drawing the score and character select title
//
void Game::DrawHeading(const std::string& headingText, const IntVector3& drawCoords, const Vector3& alignment)
{
	VoxelFontDraw_t options;
	options.mode = VOXEL_FONT_FILL_NONE;
	options.glyphColors.push_back(Rgba::WHITE);
	options.font = s_instance->m_hudFont;
	options.scale = IntVector3(1, 1, 4);
	options.borderThickness = 0;
	options.alignment = alignment;

	VoxelGrid* grid = s_instance->m_voxelGrid;
	grid->DrawVoxelText(headingText, drawCoords, options);
}


//-----------------------------------------------------------------------------------------------
// Returns true if all active players have chosen a character and are initialized as such
//
bool Game::AreAllPlayersInitialized()
{
	Player** players = s_instance->m_players;

	for (int i = 0; i < MAX_PLAYERS; ++i)
	{
		if (players[i] != nullptr)
		{
			if (players[i]->GetEntityDefinition()->GetName() == "PlayerUninitialized")
			{
				return false;
			}
		}
	}

	return true;
}


//-----------------------------------------------------------------------------------------------
// Returns the difficulty scale for the current number of players
//
void Game::RescaleDifficultyBasedOnCurrentPlayerCount()
{
	int playerCount = GetCurrentPlayerCount();
	float newDifficultyScale = 1.f;

	switch (playerCount)
	{
	case 1:
		newDifficultyScale = 1.f;
		break;
	case 2: 
		newDifficultyScale = 2.f;
		break;
	case 3:
		newDifficultyScale = 3.f;
		break;
	case 4:
		newDifficultyScale = 4.f;
		break;
	default:
		ERROR_AND_DIE("Bad player count");
		break;
	}

	s_instance->m_campaignManager->RescaleToNewDifficulty(newDifficultyScale);
}


//-----------------------------------------------------------------------------------------------
// Plays the given background music
//
void Game::PlayBGM(const std::string& filename, bool fadeIn /*= true*/, bool loop /*= true*/)
{
	AudioSystem* audio = AudioSystem::GetInstance();

	SoundID nextSong = AudioSystem::GetInstance()->CreateOrGetSound(filename);

	if (fadeIn)
	{		
		if (s_instance->m_trackTendingToZero != MISSING_SOUND_ID)
		{
			audio->StopSound(s_instance->m_trackTendingToZero);
		}

		s_instance->m_trackTendingToZero = s_instance->m_trackTendingToTarget;
		s_instance->m_tendingZeroCurrentVolume = s_instance->m_tendingTargetCurrentVolume;

		s_instance->m_tendingTargetCurrentVolume = 0.f;
		s_instance->m_trackTendingToTarget = audio->PlaySound(nextSong, loop, s_instance->m_tendingTargetCurrentVolume);
	}
	else
	{
		if (s_instance->m_trackTendingToZero != MISSING_SOUND_ID)
		{
			audio->StopSound(s_instance->m_trackTendingToZero);
			s_instance->m_trackTendingToZero = MISSING_SOUND_ID;
			s_instance->m_tendingZeroCurrentVolume = 0.f;
		}

		if (s_instance->m_trackTendingToTarget != MISSING_SOUND_ID)
		{
			audio->StopSound(s_instance->m_trackTendingToTarget);
		}

		s_instance->m_tendingTargetCurrentVolume = s_instance->m_targetMusicVolume;
		s_instance->m_trackTendingToTarget = audio->PlaySound(nextSong, loop, s_instance->m_tendingTargetCurrentVolume);
	}
}


//-----------------------------------------------------------------------------------------------
// Sets the volume that the background music should be at, and will tranition towards it
// If transitionTo is false, the bgm is immediately set to that volume
//
void Game::SetBGMVolume(float newVolume, bool transitionTo /*= true*/)
{
	s_instance->m_targetMusicVolume = newVolume;

	if (!transitionTo)
	{
		s_instance->m_tendingTargetCurrentVolume = newVolume;
		AudioSystem::GetInstance()->SetSoundPlaybackVolume(s_instance->m_trackTendingToTarget, s_instance->m_targetMusicVolume);
	}
}


//-----------------------------------------------------------------------------------------------
// Plays the system sound given by the systemSoundName
//
void Game::PlaySystemSound(const std::string& systemSoundName)
{
	bool soundExists = s_instance->m_systemSounds.find(systemSoundName) != s_instance->m_systemSounds.end();

	if (soundExists)
	{
		AudioSystem* audio = AudioSystem::GetInstance();
		audio->PlaySound(s_instance->m_systemSounds[systemSoundName]);
	}
}


//-----------------------------------------------------------------------------------------------
// Returns the singleton Game instance
//
Game* Game::GetInstance()
{
	return s_instance;
}
