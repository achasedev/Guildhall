/************************************************************************/
/* File: PlayState_Rest.cpp
/* Author: Andrew Chase
/* Date: October 24th 2018
/* Description: Implementation of the Rest PlayState
/************************************************************************/
#include "Game/Entity/Player.hpp"
#include "Game/Framework/Game.hpp"
#include "Game/Framework/World.hpp"
#include "Game/Framework/VoxelFont.hpp"
#include "Game/Framework/GameCamera.hpp"
#include "Game/Framework/CampaignManager.hpp"
#include "Game/GameStates/GameState_Playing.hpp"
#include "Game/Entity/Components/PhysicsComponent.hpp"
#include "Game/Framework/PlayStates/PlayState_Rest.hpp"
#include "Game/Framework/PlayStates/PlayState_Stage.hpp"
#include "Game/Framework/PlayStates/PlayState_Pause.hpp"
#include "Game/Framework/PlayStates/PlayState_ControllerConnect.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Time/Clock.hpp"

#define TRANSITION_EDGE_SIZE (12)


//- C FUNCTION ----------------------------------------------------------------------------------
// Returns the player's position in the transition map before the transition is finished
//
Vector3 GetTransitionPosition(Player* player, eTransitionEdge enterEdge)
{
	Vector3 position = player->GetPosition();
	Vector3 worldDimensions = Vector3(Game::GetWorld()->GetDimensions());

	switch (enterEdge)
	{
	case EDGE_NORTH:
		position += Vector3(0.f, 0.f, worldDimensions.z);
		break;
	case EDGE_SOUTH:
		position -= Vector3(0.f, 0.f, worldDimensions.z);
		break;
	case EDGE_EAST:
		position += Vector3(worldDimensions.x, 0.f, 0.f);
		break;
	case EDGE_WEST:
		position -= Vector3(worldDimensions.x, 0.f, 0.f);
		break;
	default:
		break;
	}

	return position;
}


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
// Returns whether the given position is over the edge
//
bool IsPositionInEdge(const Vector3& position, eTransitionEdge edge)
{
	Vector3 worldDimensions = Vector3(Game::GetWorld()->GetDimensions());

	float diff = 0.f;
	switch (edge)
	{
	case EDGE_NORTH:
		diff = worldDimensions.z - position.z;
		break;
	case EDGE_SOUTH:
		diff = position.z;
		break;
	case EDGE_EAST:
		diff = worldDimensions.x - position.x;
		break;
	case EDGE_WEST:
		diff = position.x;
		break;
	default:
		break;
	}

	return (diff < TRANSITION_EDGE_SIZE);
}


//- C FUNCTION ----------------------------------------------------------------------------------
// Returns whether all the players are over the threshold of the trigger, given the edge to exit over
//
bool AreAllPlayersInExitEdge(eTransitionEdge edge)
{
	Player** players = Game::GetPlayers();

	for (int i = 0; i < MAX_PLAYERS; ++i)
	{
		if (players[i] != nullptr)
		{
			Vector3 position = players[i]->GetPosition();
			
			if (!IsPositionInEdge(position, edge))
			{
				return false;
			}
		}
	}

	return true;
}


//- C FUNCTION ----------------------------------------------------------------------------------
// Returns whether the given player should keep moving over the edge during a transition
//
bool ShouldPlayerKeepMoving(Player* player, eTransitionEdge enterEdge)
{
	Vector3 transitionPosition = GetTransitionPosition(player, enterEdge);
	Vector3 worldDimensions = Vector3(Game::GetWorld()->GetDimensions());

	float diff = 0.f;

	switch (enterEdge)
	{
	case EDGE_NORTH:
		diff = worldDimensions.z - transitionPosition.z;
		break;
	case EDGE_SOUTH:
		diff = transitionPosition.z;
		break;
	case EDGE_EAST:
		diff = worldDimensions.x - transitionPosition.x;
		break;
	case EDGE_WEST:
		diff = transitionPosition.x;
		break;
	default:
		break;
	}

	return (diff < TRANSITION_EDGE_SIZE);
}


//- C FUNCTION ----------------------------------------------------------------------------------
// Returns the movement direction for a player when transitioning into the given edge
//
Vector2 GetTransitionDirectionForEnterEdge(eTransitionEdge enterEdge)
{
	Vector2 direction = Vector2::ZERO;

	switch (enterEdge)
	{
	case EDGE_NORTH:
		direction = Vector2::MINUS_Y_AXIS;
		break;
	case EDGE_SOUTH:
		direction = Vector2::Y_AXIS;
		break;
	case EDGE_EAST:
		direction = Vector2::MINUS_X_AXIS;
		break;
	case EDGE_WEST:
		direction = Vector2::X_AXIS;
		break;
	default:
		break;
	}

	return direction;
}


//-----------------------------------------------------------------------------------------------
// Gets the player's correct height duration a transition and sets it
//
void UpdatePlayerHeightForTransition(Player* player, World* transitionWorld, eTransitionEdge enterEdge)
{
	World* currWorld = Game::GetWorld();
	IntVector3 worldDimensions = currWorld->GetDimensions();

	IntVector3 transitionCoord = player->GetCoordinatePosition();
	
	switch (enterEdge)
	{
	case EDGE_NORTH:
		transitionCoord.z += worldDimensions.z;
		break;
	case EDGE_SOUTH:
		transitionCoord.z -= worldDimensions.z;
		break;
	case EDGE_EAST:
		transitionCoord.x += worldDimensions.x;
		break;
	case EDGE_WEST:
		transitionCoord.x -= worldDimensions.x;
		break;
	default:
		break;
	}

	IntVector3 bounds = player->GetOrientedDimensions();

	int currMaxHeight = currWorld->GetMapHeightForEntity(player);
	int transitionMaxHeight = transitionWorld->GetMapHeightForBounds(transitionCoord, bounds.xz());
	int finalHeight = MaxInt(currMaxHeight, transitionMaxHeight);

	Vector3 oldPosition = player->GetPosition();
	Vector3 newPosition = Vector3(oldPosition.x, (float)finalHeight, oldPosition.z);

	player->SetPosition(newPosition);
}


//- C FUNCTION ----------------------------------------------------------------------------------
// Returns the offsets used to render each map's map within the single grid
//
void GetmapOffsets(IntVector3& out_currOffset, IntVector3& out_transitionOffset, float normalizedTime, eTransitionEdge enterEdge)
{
	IntVector3 worldDimensions = Game::GetWorld()->GetDimensions();

	int offset = (int)(normalizedTime * (float)worldDimensions.x); // World is square, so can just use x

	switch (enterEdge)
	{
	case EDGE_NORTH:
		out_currOffset = IntVector3(0, 0, offset);
		out_transitionOffset = IntVector3(0, 0, offset - worldDimensions.z);
		break;
	case EDGE_SOUTH:
		out_currOffset = IntVector3(0, 0, -offset);
		out_transitionOffset = IntVector3(0, 0, worldDimensions.z - offset);
		break;
	case EDGE_EAST:
		out_currOffset = IntVector3(offset, 0, 0);
		out_transitionOffset = IntVector3(offset - worldDimensions.x, 0, 0);
		break;
	case EDGE_WEST:
		out_currOffset = IntVector3(-offset, 0, 0);
		out_transitionOffset = IntVector3(worldDimensions.x - offset, 0, 0);
		break;
	default:
		break;
	}
}


//-----------------------------------------------------------------------------------------------
// Constructor
//
PlayState_Rest::PlayState_Rest()
	: PlayState(REST_TRANSITION_IN_TIME, REST_TRANSITION_OUT_TIME)
{
	// Get the next stage and initialize the next world to enter
	CampaignStageData* nextStage = Game::GetCampaignManager()->GetNextStage();
	m_worldToTransitionTo = new World();
	m_worldToTransitionTo->InititalizeForStage(nextStage);
	m_edgeToEnter = m_worldToTransitionTo->GetDirectionToEnter();
	m_edgeToExit = GetEdgeToExit(m_edgeToEnter);
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
	if (m_state != TRANSITION_STATE_LEAVING)
	{
		int pauseIndex = CheckForPause();

		if (pauseIndex != -1)
		{
			m_gameState->PushOverrideState(new PlayState_Pause(Game::GetPlayers()[pauseIndex]));
		}
		else
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
	}
}


//-----------------------------------------------------------------------------------------------
// Enter update step
//
bool PlayState_Rest::Enter()
{
	// Respawn the dead players immediately, to avoid waiting
	Player** players = Game::GetPlayers();

	for (int i = 0; i < MAX_PLAYERS; ++i)
	{
		if (players[i] != nullptr && players[i]->IsMarkedForDelete())
		{
			players[i]->Respawn();
		}
	}

	UpdateWorldAndCamera();

	// Move the next stage text down
	float t = ClampFloatZeroToOne(m_transitionTimer.GetElapsedTimeNormalized());
	m_nextScreenTextAnchor = Interpolate(m_nextScreenTextStart, m_nextScreenTextTarget, t);

	return m_transitionTimer.HasIntervalElapsed();
}


//-----------------------------------------------------------------------------------------------
// Update
//
void PlayState_Rest::Update()
{
	// Check the controllers
	m_gameState->PerformControllerCheck();

	// Check if the players are near the move location
	bool playersReady = AreAllPlayersInExitEdge(m_edgeToExit) && Game::AreAllPlayersInitialized();

	if (playersReady)
	{
		Game::GetWorld()->SetBlockEdgeCollision(false);
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
			players[i]->GetPhysicsComponent()->SetGravity(false);

			// Check if the player is far enough into the new map space
			bool shouldMove = ShouldPlayerKeepMoving(players[i], m_edgeToEnter);

			if (shouldMove)
			{
				players[i]->Move(GetTransitionDirectionForEnterEdge(m_edgeToEnter));
			}
			else
			{
				players[i]->Decelerate();
			}
		}
	}

	UpdateWorldAndCamera();

	for (int i = 0; i < MAX_PLAYERS; ++i)
	{
		if (players[i] != nullptr)
		{
			UpdatePlayerHeightForTransition(players[i], m_worldToTransitionTo, m_edgeToEnter);
		}
	}

	if (m_transitionTimer.HasIntervalElapsed())
	{
		Game::GetWorld()->SetBlockEdgeCollision(true); // World gets deleted anyways...
		Game::SetWorld(m_worldToTransitionTo);
		Vector3 worldDimensions = Vector3(Game::GetWorld()->GetDimensions());

		// Set the player's positions to be correct on the next map
		for (int i = 0; i < MAX_PLAYERS; ++i)
		{
			if (players[i] != nullptr)
			{
				Game::GetWorld()->AddEntity(players[i]);
				players[i]->SetPosition(GetTransitionPosition(players[i], m_edgeToEnter));
				players[i]->GetPhysicsComponent()->SetGravity(true);
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
	Render();
}


//---C FUNCTION----------------------------------------------------------------------------------
// Returns the character representation of the edge for exiting
// Used for rendering the "GO!" text segment
//
char GetArrowForExitEdge(eTransitionEdge edge)
{
	switch (edge)
	{
	case EDGE_NORTH:
		return 1;
		break;
	case EDGE_SOUTH:
		return 3;
		break;
	case EDGE_EAST:
		return 4;
		break;
	case EDGE_WEST:
		return 2;
		break;
	default:
		return 0;
		break;
	}
}


//---C FUNCTION----------------------------------------------------------------------------------
// Draws the arrow directing the players towards the exit edge of the screen
//
void DrawArrow(const IntVector3 drawPosition, eTransitionEdge exitEdge, VoxelFontDraw_t& options)
{
	char arrowChar = GetArrowForExitEdge(exitEdge);

	options.colorFunction = GetColorForWaveEffect;

	VoxelFontColorWaveArgs_t colorArgs;

	switch (arrowChar)
	{
	case 1:
		colorArgs.direction = IntVector3(0, 0, 1);
		break;
	case 2:
		colorArgs.direction = IntVector3(-1, 0, 0);
		break;
	case 3:
		colorArgs.direction = IntVector3(0, 0, -1);
		break;
	case 4:
		colorArgs.direction = IntVector3(1, 0, 0);
		break;
	default:
		break;
	}

	colorArgs.speed = 2.f;

	options.colorFunctionArgs = &colorArgs;

	std::string arrow = "";
	arrow += arrowChar;
	options.scale = IntVector3(4, 4, 1);
	options.up = IntVector3(0, 0, 1);
	Game::GetVoxelGrid()->DrawVoxelText(arrow, drawPosition, options);
}


//---C FUNCTION----------------------------------------------------------------------------------
// Shades the edge of the map that the players need to walk into in order to move the screen
// to the next stage
//
void ShadeExitEdge(eTransitionEdge exitEdge)
{
	World* world = Game::GetWorld();
	IntVector3 worldDimensions = world->GetDimensions();

	// Set up our iteration limits
	int startX = 0;
	int startZ = 0;
	int endX = 0;
	int endZ = 0;

	switch (exitEdge)
	{
	case EDGE_NORTH:
		startX = 0;
		endX = worldDimensions.x;

		startZ = worldDimensions.z - TRANSITION_EDGE_SIZE;
		endZ = worldDimensions.z;
		break;
	case EDGE_SOUTH:
		startX = 0;
		endX = worldDimensions.x;

		startZ = 0;
		endZ = TRANSITION_EDGE_SIZE;
		break;
	case EDGE_EAST:
		startX = worldDimensions.x - TRANSITION_EDGE_SIZE;
		endX = worldDimensions.x;

		startZ = 0;
		endZ = worldDimensions.z;
		break;
	case EDGE_WEST:
		startX = 0;
		endX = TRANSITION_EDGE_SIZE;

		startZ = 0;
		endZ = worldDimensions.z;
		break;
	default:
		break;
	}

	// Calculate the flash color
	float time = 4.f * Game::GetGameClock()->GetTotalSeconds();
	float t = ModFloat(time, 2.0f);

	if (t > 1.0f)
	{
		t = 2.0f - t;
	}

	Rgba flashColor = Interpolate(Rgba::WHITE, Rgba::CYAN, t);

	for (int zIndex = startZ; zIndex < endZ; ++zIndex)
	{
		for (int xIndex = startX; xIndex < endX; ++xIndex)
		{
			IntVector3 coords = IntVector3(xIndex, 0, zIndex);
			coords.y = world->GetGroundElevationAtCoord(coords.xz());

			Game::GetVoxelGrid()->ColorVoxelAtCoords(coords, flashColor);
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Renders the state during normal updating
//
void PlayState_Rest::Render() const
{
	Game::GetWorld()->DrawToGrid();
	Game::DrawPlayerHUD();

	// Don't draw "GO" if a player is still choosing a character
	if (Game::AreAllPlayersInitialized())
	{
		// Draw the next screen text
		VoxelFontDraw_t options;
		options.alignment = Vector3(0.5f);
		options.font = Game::GetMenuFont();
		options.glyphColors.push_back(Rgba::BLUE);
		options.scale = IntVector3(3, 3, 1);

		Game::GetVoxelGrid()->DrawVoxelText("GO", m_nextScreenTextAnchor, options);

		IntVector3 arrowCoords = m_nextScreenTextAnchor - IntVector3(0, 4 * options.font->GetGlyphDimensions().y, 0);
		DrawArrow(arrowCoords, m_edgeToExit, options);

		if (m_transitionTimer.HasIntervalElapsed())
		{
			ShadeExitEdge(m_edgeToExit);
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Renders the leave transition
//
void PlayState_Rest::Render_Leave() const
{
	IntVector3 currOffset;
	IntVector3 transitionOffset;

	GetmapOffsets(currOffset, transitionOffset, m_transitionTimer.GetElapsedTimeNormalized(), m_edgeToEnter);

	Game::GetWorld()->DrawToGridWithOffset(currOffset);
	m_worldToTransitionTo->DrawToGridWithOffset(transitionOffset);

	Game::DrawPlayerHUD();

	if (Game::AreAllPlayersInitialized())
	{
		// Draw the next screen text with the offset
		VoxelFontDraw_t options;
		options.alignment = Vector3(0.5f);
		options.font = Game::GetMenuFont();
		options.glyphColors.push_back(Rgba::BLUE);
		options.scale = IntVector3(3, 3, 1);

		Game::GetVoxelGrid()->DrawVoxelText("GO", m_nextScreenTextAnchor + currOffset, options);

		IntVector3 arrowCoords = m_nextScreenTextAnchor - IntVector3(0, 4 * options.font->GetGlyphDimensions().y, 0);
		DrawArrow(arrowCoords + currOffset, m_edgeToExit, options);
	}
}
