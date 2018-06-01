/************************************************************************/
/* File: PlayAction_Move.cpp
/* Author: Andrew Chase
/* Date: March 12th, 2018
/* Description: Implementation of the move PlayAction
/************************************************************************/
#include "Game/Map.hpp"
#include "Game/Game.hpp"
#include "Game/Actor.hpp"
#include "Game/GameCommon.hpp"
#include "Game/BoardState.hpp"
#include "Game/PlayAction_Jump.hpp"
#include "Game/PlayAction_Move.hpp"
#include "Game/PlayAction_Walk.hpp"
#include "Game/ActorController.hpp"
#include "Game/GameState_Playing.hpp"
#include "Game/PlayMode_SelectTile.hpp"
#include "Engine/Core/HeatMap.hpp"
#include "Engine/Math/MathUtils.hpp"


//-----------------------------------------------------------------------------------------------
// Constructor - Takes the controller for the acting actor
//
PlayAction_Move::PlayAction_Move(ActorController* actorController)
	: PlayAction(ACTION_CLASS_MOVE)
	, m_controller(actorController)
{
}


//-----------------------------------------------------------------------------------------------
// Intitializes the action by pushing a select tile mode for finding the destination
//
void PlayAction_Move::Setup()
{
	// Find the traversable coords
	BoardState* boardState = Game::GetCurrentBoardState();
	Actor* actor = m_controller->GetActor();
	m_currCoords = actor->GetMapCoordinate();

	std::vector<IntVector3> traversibleCoords = boardState->GetTraversableCoords(actor->GetMapCoordinate(), actor->GetMoveSpeed(), actor->GetJumpHeight(), actor->GetTeamIndex(), m_distanceMap);

	// Push a new mode for target selection
	GameState_Playing* playState = Game::GetGameStatePlaying();

	TileSelectionCallback callback = std::bind(&PlayAction_Move::OnTileSelection, this, std::placeholders::_1, std::placeholders::_2);
	playState->PushMode(new PlayMode_SelectTile(m_controller, traversibleCoords, callback));

	m_actionState = ACTION_STATE_READY;
}


//-----------------------------------------------------------------------------------------------
// Pushes all the walk/jump animations necessary for the move
//
void PlayAction_Move::Start()
{
	Actor* actor = m_controller->GetActor();
	Map* map = Game::GetCurrentBoardState()->GetMap();

	while (m_movePath.size() > 0)
	{
		// Get the next step in the path
		IntVector3 nextTargetCoord = m_movePath.top();
		Vector3 nextTargetPosition = map->MapCoordsToWorldPosition(nextTargetCoord);

		if (nextTargetCoord.z != m_currCoords.z)
		{
			// If the next step is on a different elevation, then jump
			PlayAction_Jump* jumpAction = new PlayAction_Jump(actor, nextTargetPosition);
			GameState_Playing* playState = Game::GetGameStatePlaying();
			jumpAction->Setup();
			playState->EnqueueAction(jumpAction);
		}
		else
		{
			// Else the next space is the same elevation, so walk, don't jump
			PlayAction_Walk* walkAction = new PlayAction_Walk(actor, nextTargetPosition);
			GameState_Playing* playState = Game::GetGameStatePlaying();
			walkAction->Setup();
			playState->EnqueueAction(walkAction);
		}

		// Pop the target space and update current coordinate
		m_movePath.pop();
		m_currCoords = nextTargetCoord;
	}
	

	// Check if we've reached the end, if so clean up
	actor->AddToWaitTime(60.f);
	
	m_actionState = ACTION_STATE_RUNNING;
}


//-----------------------------------------------------------------------------------------------
// Moves the character to the destination
//
void PlayAction_Move::Update()
{
	m_actionState = ACTION_STATE_FINISHED;
}


//-----------------------------------------------------------------------------------------------
// Draws the world space elements
//
void PlayAction_Move::RenderWorldSpace() const
{
}


//-----------------------------------------------------------------------------------------------
// Draws the screen space elements
//
void PlayAction_Move::RenderScreenSpace() const
{
}


//-----------------------------------------------------------------------------------------------
// Called before deletion of this action
//
void PlayAction_Move::Exit()
{
	// Tell the controller we moved
	if (m_actionState == ACTION_STATE_FINISHED)
	{
		m_controller->SetControllerFlag(m_actionClass, true);
	}

	if (m_distanceMap != nullptr)
	{
		delete m_distanceMap;
		m_distanceMap = nullptr;
	}
}


//-----------------------------------------------------------------------------------------------
// Callback for when a tile has been selected (or selection was cancelled)
//
void PlayAction_Move::OnTileSelection(bool wasCancelled, const IntVector3& selection)
{
	if (wasCancelled)
	{
		m_actionState = ACTION_STATE_CANCELLED;
	}
	else
	{
		// Reconstruct the path using the heatmap
		m_targetCoords = selection;
		ConstructMovePath();

		m_actionState = ACTION_STATE_READY;
	}
}


//-----------------------------------------------------------------------------------------------
// Finds the path from the actor's current position to the target position using the distance map
// Start space is not included in the path
//
void PlayAction_Move::ConstructMovePath()
{
	// Ensure the path is empty
	while (m_movePath.size() > 0)
	{
		m_movePath.pop();
	}

	IntVector3 currCoords = m_targetCoords;

	IntVector3 actorCoord = m_controller->GetActor()->GetMapCoordinate();
	DebuggerPrintf("Actor Coord: (%i, %i, %i)\n", actorCoord.x, actorCoord.y, actorCoord.z);

	while (actorCoord != currCoords)
	{
		DebuggerPrintf("Path Coord: (%i, %i, %i)\n", currCoords.x, currCoords.y, currCoords.z);
		m_movePath.push(currCoords);
		currCoords = GetMinNeighbor(currCoords);
	}
}


//-----------------------------------------------------------------------------------------------
// Returns the neighbor coords with the minimum cost relative to the currCoords
//
IntVector3 PlayAction_Move::GetMinNeighbor(const IntVector3& currCoords) const
{
	Map* map = Game::GetCurrentBoardState()->GetMap();

	// Setup initial distances
	float northDistance = 9999999.f;
	float southDistance = 9999999.f;
	float eastDistance	= 9999999.f;
	float westDistance	= 9999999.f;

	// Find the coords
	IntVector3 northCoords	= map->GetNeighborCoords(currCoords, IntVector2::STEP_NORTH);
	IntVector3 southCoords	= map->GetNeighborCoords(currCoords, IntVector2::STEP_SOUTH);
	IntVector3 eastCoords	= map->GetNeighborCoords(currCoords, IntVector2::STEP_EAST);
	IntVector3 westCoords	= map->GetNeighborCoords(currCoords, IntVector2::STEP_WEST);

	// Update to actual distances on valid coords
	if (map->Are2DCoordsValid(northCoords)) { northDistance	= m_distanceMap->GetHeat(northCoords.xy()); }
	if (map->Are2DCoordsValid(southCoords)) { southDistance	= m_distanceMap->GetHeat(southCoords.xy()); }
	if (map->Are2DCoordsValid(eastCoords))  { eastDistance	= m_distanceMap->GetHeat(eastCoords.xy());  }
	if (map->Are2DCoordsValid(westCoords))  { westDistance	= m_distanceMap->GetHeat(westCoords.xy());  }

	// Return the minimum distance coordinate
	float minDistance = MinFloat(northDistance, southDistance, eastDistance, westDistance);

	if		(minDistance == northDistance)	{ return northCoords; }
	else if (minDistance == southDistance)	{ return southCoords; }
	else if (minDistance == eastDistance)	{ return eastCoords;  }
	else									{ return westCoords;  }
}
