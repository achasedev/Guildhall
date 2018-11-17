/************************************************************************/
/* File: PlayState_Rest.cpp
/* Author: Andrew Chase
/* Date: October 24th 2018
/* Description: Implementation of the Rest PlayState
/************************************************************************/
#include "Game/Entity/Player.hpp"
#include "Game/Framework/Game.hpp"
#include "Game/Framework/World.hpp"
#include "Game/Framework/GameCamera.hpp"
#include "Game/Framework/CampaignManager.hpp"
#include "Game/GameStates/GameState_Playing.hpp"
#include "Game/Framework/PlayStates/PlayState_Rest.hpp"
#include "Engine/Math/MathUtils.hpp"

// For debug rendering
#include "Engine/Core/Window.hpp"
#include "Engine/Core/Utility/StringUtils.hpp"
#include "Engine/Rendering/DebugRendering/DebugRenderSystem.hpp"

#define TRANSITION_MOVE_OFFSET (10)


//- C FUNCTION ----------------------------------------------------------------------------------
// Returns the edge to leave over the current map given the edge we need to enter on the next map
//
eTransitionEdge GetEdgeToExit(eTransitionEdge edgeToEnter)
{
	switch (edgeToEnter)
	{
	case EDGE_NORTH:
		return EDGE_SOUTH;
		break;
	case EDGE_SOUTH:
		return EDGE_NORTH;
		break;
	case EDGE_EAST:
		return EDGE_WEST;
		break;
	case EDGE_WEST:
		return EDGE_EAST;
		break;
	default:
		return EDGE_WEST;
		break;
	}
}


//- C FUNCTION ----------------------------------------------------------------------------------
// Returns whether all the players are over the threshold of the trigger, given the edge to exit over
//
bool AreAllPlayersOverTransitionEdge(eTransitionEdge edge)
{
	Player** players = Game::GetPlayers();
	Vector3 dimensions = Vector3(Game::GetWorld()->GetDimensions());

	for (int i = 0; i < MAX_PLAYERS; ++i)
	{
		if (players[i] != nullptr)
		{
			Vector3 position = players[i]->GetPosition();
			
			float diff = 0.f;
			switch (edge)
			{
			case EDGE_NORTH:
				diff = dimensions.z - position.z;
				break;
			case EDGE_SOUTH:
				diff = position.z;
				break;
			case EDGE_EAST:
				diff = dimensions.x - position.x;
				break;
			case EDGE_WEST:
				diff = position.x;
				break;
			default:
				break;
			}

			if (diff > TRANSITION_MOVE_OFFSET)
			{
				return false;
			}
		}
	}

	return true;
}


//-----------------------------------------------------------------------------------------------
// Constructor
//
PlayState_Rest::PlayState_Rest()
	: PlayState(REST_TRANSITION_IN_TIME, REST_TRANSITION_OUT_TIME)
{
}


//-----------------------------------------------------------------------------------------------
// Destructor
//
PlayState_Rest::~PlayState_Rest()
{

}


//-----------------------------------------------------------------------------------------------
// Checks for gameplay input on the players
//
void PlayState_Rest::ProcessInput()
{
	// Check player input
	Player** players = Game::GetPlayers();

	for (int i = 0; i < MAX_PLAYERS; ++i)
	{
		if (Game::IsPlayerAlive(i))
		{
			players[i]->ProcessGameplayInput();
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Enter update step
//
bool PlayState_Rest::Enter()
{
	// Respawn the dead players
	Player** players = Game::GetPlayers();

	for (int i = 0; i < MAX_PLAYERS; ++i)
	{
		if (players[i] != nullptr && players[i]->IsMarkedForDelete())
		{
			players[i]->Respawn();
		}
	}

	UpdateWorldAndCamera();

	if (m_transitionTimer.HasIntervalElapsed())
	{
		// Get the next stage and initialize the next world to enter
		CampaignStage* nextStage = Game::GetCampaignManager()->GetNextStage();
		m_worldToTransitionTo = new World();
		m_worldToTransitionTo->InititalizeForStage(nextStage);
		//m_edgeToEnter = m_worldToTransitionTo->GetDirectionToEnter();
		//m_edgeToExit = GetEdgeToExit(m_edgeToEnter);

		return true;
	}

	return false;
}


//-----------------------------------------------------------------------------------------------
// Update
//
void PlayState_Rest::Update()
{
	// Check if the players are near the move location
	bool playersReady = AreAllPlayersOverTransitionEdge(EDGE_WEST);

	if (playersReady)
	{
		m_gameState->TransitionToPlayState(new PlayState_Stage());
	}
	else
	{
		UpdateWorldAndCamera();
	}
}


//-----------------------------------------------------------------------------------------------
// Leave update
//
bool PlayState_Rest::Leave()
{
	// Move the players off the map as it's transitioning
	Player** players = Game::GetPlayers();
	
	for (int i = 0; i < MAX_PLAYERS; ++i)
	{
		if (players[i] != nullptr)
		{
			// Check if the player is far enough into the new map space
			Vector3 oldPos = players[i]->GetPosition();
			Vector3 newPos = oldPos + Vector3(256.f, 0.f, 0.f);

			if (((float) m_worldToTransitionTo->GetDimensions().x - newPos.x) < TRANSITION_MOVE_OFFSET)
			{
				players[i]->Move(Vector2::DIRECTION_LEFT);
			}
		}
	}

	UpdateWorldAndCamera();

	for (int i = 0; i < MAX_PLAYERS; ++i)
	{
		if (players[i] != nullptr)
		{
			if (!Game::GetWorld()->IsEntityOnMap(players[i]))
			{
				float currMaxHeight = Game::GetWorld()->GetMapHeightForEntity(players[i]);

				IntVector3 transitionCoord = players[i]->GetCoordinatePosition() + IntVector3(256, 0, 0);
				IntVector3 bounds = players[i]->GetDimensions();
				float transitionMaxHeight = Game::GetWorld()->GetMapHeightForBounds(transitionCoord, bounds.xz());

				float finalHeight = MaxFloat(currMaxHeight, transitionMaxHeight);

				Vector3 oldPosition = players[i]->GetPosition();
				Vector3 newPosition = Vector3(oldPosition.x, finalHeight, oldPosition.z);

				players[i]->SetPosition(newPosition);
			}
		}
	}

	if (m_transitionTimer.HasIntervalElapsed())
	{
		Game::SetWorld(m_worldToTransitionTo);

		// Set the player's positions to be correct on the next map
		for (int i = 0; i < MAX_PLAYERS; ++i)
		{
			if (players[i] != nullptr)
			{
				Vector3 oldPos = players[i]->GetPosition();
				Vector3 newPos = oldPos + Vector3(256.f, 0.f, 0.f);

				players[i]->SetPosition(newPos);
			}
		}

		return true;
	}
	
	return false;
}


//-----------------------------------------------------------------------------------------------
// Renders the enter transition
//
void PlayState_Rest::Render_Enter() const
{
	Game::GetWorld()->DrawToGrid();
	DebugRenderSystem::Draw2DText(Stringf("Rest Enter: %.2f seconds remaining", m_transitionTimer.GetTimeUntilIntervalEnds()), Window::GetInstance()->GetWindowBounds(), 0.f);
}


//-----------------------------------------------------------------------------------------------
// Renders the state during normal updating
//
void PlayState_Rest::Render() const
{
	Game::GetWorld()->DrawToGrid();
	DebugRenderSystem::Draw2DText(Stringf("Rest - GOGOGO"), Window::GetInstance()->GetWindowBounds(), 0.f);
}


//-----------------------------------------------------------------------------------------------
// Renders the leave transition
//
void PlayState_Rest::Render_Leave() const
{
	// For testing, always enter from the west
	int xOffset = (int)(m_transitionTimer.GetElapsedTimeNormalized() * 256.f);

	Game::GetWorld()->DrawToGridWithOffset(IntVector3(xOffset, 0, 0));
	m_worldToTransitionTo->DrawToGridWithOffset(IntVector3(xOffset - 256, 0, 0));

	DebugRenderSystem::Draw2DText(Stringf("Rest Leave: %.2f seconds remaining", m_transitionTimer.GetTimeUntilIntervalEnds()), Window::GetInstance()->GetWindowBounds(), 0.f);
}
