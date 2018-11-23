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

#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Time/Clock.hpp"
#include "Engine/Rendering/Core/Camera.hpp"
#include "Engine/Rendering/Core/Renderer.hpp"
#include "Engine/Rendering/Core/RenderScene.hpp"
#include "Engine/Rendering/DebugRendering/DebugRenderSystem.hpp"
#include "Engine/Core/DeveloperConsole/Command.hpp"
#include "Engine/Core/Utility/XmlUtilities.hpp"


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

	for (int i = 0; i < MAX_PLAYERS; ++i)
	{
		m_players[i] = nullptr;
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

	Command::Register("killall", "Kills all entities", Command_KillAll);
}


//-----------------------------------------------------------------------------------------------
// Deletes the singleton instance
//
void Game::ShutDown()
{
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
}


//-----------------------------------------------------------------------------------------------
// Draws to screen
//
void Game::Render() const
{
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
// Returns the world used for all entities and terrain
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
// Returns the Campaign Manager for the game
//
CampaignManager* Game::GetCampaignManager()
{
	return s_instance->m_campaignManager;
}


//-----------------------------------------------------------------------------------------------
// Returns whether the player at the given index exists and is not marked for delete (dead)
//
bool Game::IsPlayerAlive(unsigned int index)
{
	if (index >= MAX_PLAYERS)
	{
		return false;
	}

	Player* player = s_instance->m_players[index];
	return (player != nullptr && !player->IsMarkedForDelete());
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
	IntVector3 hudStart = IntVector3(1, 35, 252);

	// HUD is on the top (players 1 and 2)
	if (id <= 1)
	{
		hudStart.y += 10;
	}

	// HUD is on the right (players 2 and 4)
	if (id % 2 == 1)
	{
		hudStart.x = 254;
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
	options.textColor = Rgba::WHITE;
	options.fillColor = Player::GetColorForPlayerID(playerID);
	options.font = font;
	options.scale = IntVector3(1, 1, 4);
	options.borderThickness = 0;

	// Parameters that depend on player ID

	options.alignment = GetHUDAlignmentForPlayerID(playerID);
	IntVector3 startCoords = GetHUDStartForPlayerID(playerID);
	IntVector3 currDrawCoords = startCoords;

	// Draw the player text
	std::string hudText = GetHUDTextForPlayerID(playerID);

	IntVector3 textCoords = currDrawCoords - IntVector3(0, 0, 2);
	grid->DrawVoxelText(hudText, textCoords, options);

	int totalHeight = 10;
	int textWidth = font->GetTextDimensions(hudText).x;

	currDrawCoords.x += (playerID % 2 == 1 ? -textWidth : textWidth);
	IntVector3 textBackgroundCoords = ((playerID % 2) == 1 ? currDrawCoords : startCoords);
	grid->DrawSolidBox(textBackgroundCoords - IntVector3(1, 1, 0), IntVector3(textWidth + 2, 10, 4), options.fillColor);


	// If the player exists, draw the rest
	if (player != nullptr)
	{
		currDrawCoords += IntVector3(playerID % 2 == 1 ? -2 : 0, 0, 0);

		int maxHealth = player->GetEntityDefinition()->GetInitialHealth();
		int currHealth = player->GetHealth();

		grid->DrawSolidBox(currDrawCoords, IntVector3(1, 8, 4), options.fillColor);
		currDrawCoords.x += (playerID % 2 == 1 ? -8 : 1);

		IntVector3 healthBoxStart = currDrawCoords;

		for (int i = 0; i < maxHealth; ++i)
		{
			Rgba color = Rgba::GRAY;

			IntVector3 currBoxStart = currDrawCoords;
			IntVector3 healthBoxDimensions = IntVector3(8, 8, 4);

			if (i < currHealth)
			{
				color = Rgba::RED;

				currBoxStart.z -= 2;
				healthBoxDimensions.z += 2;
			}
			else
			{
				currBoxStart.z += 2;
				healthBoxDimensions.z -= 2;
			}

			grid->DrawSolidBox(currBoxStart, healthBoxDimensions, color);
			
			currDrawCoords.x += (playerID % 2 == 1 ? -1 : 8);

			grid->DrawSolidBox(currDrawCoords, IntVector3(1, 8, 4), options.fillColor);
			currDrawCoords.x += (playerID % 2 == 1 ? -8 : 1);
		}


		IntVector3 borderDimensions = IntVector3(AbsoluteValue(currDrawCoords.x - healthBoxStart.x) + 1, 10, 4);

		IntVector3 borderStart = IntVector3((playerID % 2 == 1) ? currDrawCoords.x + 9 : healthBoxStart.x, startCoords.y, startCoords.z);
		grid->DrawWireBox(borderStart - IntVector3(1, 1, 0), borderDimensions, options.fillColor, true, true, false);
	}
	else
	{
		grid->DrawVoxelText("Press Start", currDrawCoords + IntVector3(0, 0, -2), options);

		IntVector3 textStart = currDrawCoords;
		currDrawCoords.x += ((playerID % 2 == 1) ? -1 : 1) * font->GetTextDimensions("Press Start").x;
	
		IntVector3 borderStart = IntVector3((playerID % 2 == 1) ? currDrawCoords.x : textStart.x, textStart.y, textStart.z);
		IntVector3 borderDimensions = IntVector3(AbsoluteValue(textStart.x - currDrawCoords.x), 10, 4);
		grid->DrawSolidBox(borderStart - IntVector3(1, 1, 0), borderDimensions, options.fillColor);
	}
}


//-----------------------------------------------------------------------------------------------
// Draws the in-game HUD that represents current player state
// Multiple game states render this, hence it being placed on Game
//
void Game::DrawInGameUI()
{
	VoxelGrid* grid = s_instance->m_voxelGrid;
	Player** players = s_instance->m_players;

	// Draw the player's health/shield bars and weapon indicators
	for (int i = 0; i < MAX_PLAYERS; ++i)
	{
		DrawHUDForPlayer(i, players[i], grid, s_instance->m_hudFont);
	}

	// Draw the score
	VoxelFontDraw_t options;
	options.mode = VOXEL_FONT_FILL_NONE;
	options.textColor = Rgba::WHITE;
	options.font = s_instance->m_hudFont;
	options.scale = IntVector3(1, 1, 4);
	options.borderThickness = 0;
	options.alignment = Vector3(0.5f, 0.f, 0.f);

	grid->DrawVoxelText("Score: 99999999", IntVector3(128, 56, 252), options);
}


//-----------------------------------------------------------------------------------------------
// Returns the singleton Game instance
//
Game* Game::GetInstance()
{
	return s_instance;
}
