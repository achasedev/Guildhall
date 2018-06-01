/************************************************************************/
/* File: PlayMode_SelectTile.cpp
/* Author: Andrew Chase
/* Date: March 12th, 2018
/* Description: Implementation of the Select Tile PlayMode
/************************************************************************/
#include "Game/Map.hpp"
#include "Game/Game.hpp"
#include "Game/Actor.hpp"
#include "Game/BoardState.hpp"
#include "Game/GameCommon.hpp"
#include "Game/ActorController.hpp"
#include "Game/GameState_Playing.hpp"
#include "Game/PlayMode_SelectTile.hpp"

#include "Engine/Input/InputSystem.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/OrbitCamera.hpp"


//-----------------------------------------------------------------------------------------------
// Single constructor, takes the controller, a list of tiles that can be selected, and the callback
//
PlayMode_SelectTile::PlayMode_SelectTile(ActorController* controller, std::vector<IntVector3>& selectableCoords, TileSelectionCallback callback)
	: PlayMode(controller)
	, m_selectableCoords(selectableCoords)
	, m_callback(callback)
{
}


//-----------------------------------------------------------------------------------------------
// Checks for input (moving the cursor and selection)
//
void PlayMode_SelectTile::ProcessInput()
{
	PlayMode::ProcessInput();
	ProcessSelectionInput();
}


//-----------------------------------------------------------------------------------------------
// Update
//
void PlayMode_SelectTile::Update()
{
	// Update the camera
	GameState_Playing* playState = Game::GetGameStatePlaying();
	Map* map = Game::GetCurrentBoardState()->GetMap();
	playState->TranslateCamera(map->MapCoordsToWorldPosition(m_cursorCoords));
}


//-----------------------------------------------------------------------------------------------
// Draws the selectable tiles
//
void PlayMode_SelectTile::RenderWorldSpace() const
{
	// Renders all tiles, including the cursor
	RenderTiles();
}


//-----------------------------------------------------------------------------------------------
// Draws any UI for this mode
//
void PlayMode_SelectTile::RenderScreenSpace() const
{
	PlayMode::RenderScreenSpace();
}


//-----------------------------------------------------------------------------------------------
// Called before deletion
//
void PlayMode_SelectTile::Exit()
{
}


//-----------------------------------------------------------------------------------------------
// Checks if the selection button was pressed
//
void PlayMode_SelectTile::ProcessSelectionInput()
{
	XboxController& controller = InputSystem::GetPlayerOneController();

	if (controller.WasButtonJustPressed(XBOX_BUTTON_A))
	{
		// If we select the current actor's position, treat it as a cancel
		Actor* actor = m_controller->GetActor();

		if (actor->GetMapCoordinate() == m_cursorCoords)
		{
			m_callback(true, IntVector3());
			m_isFinished = true;
		}

		// Check if it's a valid tile - call the callback if so
		if (IsCursorInSelection())
		{
			m_callback(false, m_cursorCoords);
			m_isFinished = true;
		}
	}

	// Cancel again if we press B
	if (controller.WasButtonJustPressed(XBOX_BUTTON_B))
	{
		m_callback(true, IntVector3::ZERO);
		m_isFinished = true;
	}
}


//-----------------------------------------------------------------------------------------------
// Returns true if the cursor is in the selection list, false otherwise
//
bool PlayMode_SelectTile::IsCursorInSelection() const
{
	return std::find(m_selectableCoords.begin(), m_selectableCoords.end(), m_cursorCoords) != m_selectableCoords.end();
}


//-----------------------------------------------------------------------------------------------
// Draws the selection tiles and cursor to screen
//
void PlayMode_SelectTile::RenderTiles() const
{
	Renderer* renderer = Renderer::GetInstance();
	renderer->BindTexture(0, "White");

	// Render the cursor, color based on location
	Rgba cursorColor = Rgba(255,0,0,200);
	if (IsCursorInSelection())
	{
		cursorColor = Rgba(255,255,0,200);
	}

	RenderSingleTile(m_cursorCoords, cursorColor);


	// Render all the selectable tiles
	Rgba selectionColor = Rgba(0,0,255,255);
	int numTiles = (int) m_selectableCoords.size();
	for (int tileIndex = 0; tileIndex < numTiles; ++tileIndex)
	{
		RenderSingleTile(m_selectableCoords[tileIndex], selectionColor);
	}

	// Render again for the x-ray tiles
	renderer->EnableDepth(COMPARE_GREATER, false);

	// Render all the selectable tiles
	selectionColor.ScaleRGB(0.8f);
	for (int tileIndex = 0; tileIndex < numTiles; ++tileIndex)
	{
		RenderSingleTile(m_selectableCoords[tileIndex], selectionColor);
	}

	// Render the cursor
	cursorColor.ScaleRGB(0.8f);
	RenderSingleTile(m_cursorCoords, cursorColor);

	renderer->EnableDepth(COMPARE_LESS, true);
}
