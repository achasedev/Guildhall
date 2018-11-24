/************************************************************************/
/* File: GameState_Ready.cpp
/* Author: Andrew Chase
/* Date: June 1st, 2018
/* Description: Implementation of the Ready GameState class
/************************************************************************/
#include "Game/Entity/Player.hpp"
#include "Game/Framework/Game.hpp"
#include "Game/Framework/World.hpp"
#include "Game/Framework/VoxelGrid.hpp"
#include "Game/Framework/VoxelFont.hpp"
#include "Game/Framework/GameCamera.hpp"
#include "Game/Framework/CampaignManager.hpp"
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
	for (int i = 0; i < MAX_PLAYERS; ++i)
	{
		m_players[i] = nullptr;
	}
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

	for (int i = 0; i < MAX_PLAYERS; ++i)
	{
		if (m_players[i] != nullptr)
		{
			m_players[i]->ProcessGameplayInput();
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Simulation update
//
void GameState_Ready::Update()
{
	CheckToAddPlayers();

	GameCamera* camera = Game::GetGameCamera();

	if (camera != nullptr && !camera->IsEjected())
	{
		//Game::GetGameCamera()->LookAt(m_defaultCameraPosition, Vector3(128.f, 32.f, 128.f));
		Game::GetGameCamera()->LookAtGridCenter();
	}

	Game::GetWorld()->Update();

	// Check if players are ready to move
	if (AreAllActivePlayersReady())
	{
		Game::TransitionToGameState(new GameState_Playing());
	}
}


//-----------------------------------------------------------------------------------------------
// Draws the ready UI elements to screen
//
void GameState_Ready::Render() const
{
	Game::GetWorld()->DrawToGrid();
	Game::DrawPlayerHUD();
	Game::DrawHeading("CHOOSE YOUR CHARACTER");

	VoxelFontDraw_t options;
	options.mode = VOXEL_FONT_FILL_NONE;
	options.textColor = Rgba::BLUE;
	options.fillColor = Rgba::BLUE;
	options.font = Game::GetMenuFont();
	options.scale = IntVector3(1, 1, 1);
	options.up = IntVector3(0, 1, 0);
	options.alignment = Vector3(0.5f, 0.5f, 0.5f);
	options.borderThickness = 0;

	Game::GetVoxelGrid()->DrawVoxelText("CHARACTER SELECT", IntVector3(128, 8, 255), options);
}


//-----------------------------------------------------------------------------------------------
// Renders the leave state transition
//
void GameState_Ready::Render_Leave() const
{
	Render();
}


//-----------------------------------------------------------------------------------------------
// Checks if players have just connected, and if so spawns them into the scene
//
void GameState_Ready::CheckToAddPlayers()
{
	Player** gamePlayers = Game::GetPlayers();

	for (int i = 0; i < MAX_PLAYERS; ++i)
	{
		if (m_players[i] == nullptr && gamePlayers[i] != nullptr)
		{
			m_players[i] = gamePlayers[i];
			Game::GetWorld()->AddEntity(m_players[i]);
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Checks if all active players have selected a character and are in the transition location
// Returns true if so
//
bool GameState_Ready::AreAllActivePlayersReady()
{
	return true;
}


//-----------------------------------------------------------------------------------------------
// Called when the game transitions into this state, before the first update
//
bool GameState_Ready::Enter()
{
	Game::GetCampaignManager()->Initialize("Data/Spawning.xml");
	Game::GetWorld()->InititalizeForStage(Game::GetCampaignManager()->GetNextStage());
	Game::GetCampaignManager()->StartNextStage();

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
