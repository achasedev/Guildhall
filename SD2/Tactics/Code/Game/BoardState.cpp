/************************************************************************/
/* File: BoardState.cpp
/* Author: Andrew Chase
/* Date: February 22nd, 2017
/* Description: Implementation of the Encounter class
/************************************************************************/
#include <string>

#include "Game/Map.hpp"
#include "Game/Game.hpp"
#include "Game/Actor.hpp"
#include "Game/BoardState.hpp"
#include "Game/GameCommon.hpp"
#include "Game/BoardStateDefinition.hpp"

#include "Engine/Core/Window.hpp"
#include "Engine/Core/HeatMap.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/OrbitCamera.hpp"


//-----------------------------------------------------------------------------------------------
// Constructor
//
BoardState::BoardState(BoardStateDefinition* definition)
	: m_definition(definition)
{
}


//-----------------------------------------------------------------------------------------------
// Destructor
//
BoardState::~BoardState()
{
	delete m_map;
	m_map = nullptr;
	DestroyAllActors();
}


//-----------------------------------------------------------------------------------------------
// Updates this encounter by updating the map and the actors
//
void BoardState::Update()
{
	m_map->Update();
	UpdateAllActors();
}


//-----------------------------------------------------------------------------------------------
// Render DEPRECATED - currently separated render functions to control draw order
//
void BoardState::Render() const
{
}


//-----------------------------------------------------------------------------------------------
// Draws the map to the screen
//
void BoardState::RenderMap() const
{
	m_map->Render();
}


//-----------------------------------------------------------------------------------------------
// Returns the map
//
Map* BoardState::GetMap() const
{
	return m_map;
}


//----------------------------------------------------------------------------------
// Updates the distance value at currIndex based on the distance value of neighborIndex
//
bool UpdateCurrFromNeighbor(int currIndex, int neighborIndex, int speed, int jump, const HeatMap* costs, const HeatMap* heights, HeatMap* distanceMap)
{
	// Bad index, just return false
	if (neighborIndex == -1) { return false; }

	// For checking if it is a shorter path
	float currDistance = distanceMap->GetHeat(currIndex);
	float newDistance = distanceMap->GetHeat(neighborIndex) + costs->GetHeat(currIndex);

	// For checking if we can jump high enough
	float heightDifference = AbsoluteValue(heights->GetHeat(currIndex) - heights->GetHeat(neighborIndex));

	// For checking if we can move that far
	bool isWithinMaxDist = (newDistance < speed);

	if (newDistance < currDistance && isWithinMaxDist && heightDifference <= (float) jump)
	{
		distanceMap->SetHeat(currIndex, newDistance);
		return true;
	} 

	return false;
}


std::vector<IntVector3> BoardState::GetTraversableCoords(const IntVector3& startBlock, int speed, int jump, int teamIndex, HeatMap*& out_distanceMap) const
{
	std::vector<IntVector3> coords;

	// Construct a cost map - for going around enemy actors
	HeatMap* costMap = new HeatMap(m_map->GetMapDimensions2D(), 1.f);

	for (int actorIndex = 0; actorIndex < (int) m_actors.size(); actorIndex++)
	{
		Actor* currActor = GetActorByIndex(actorIndex);
		if (currActor->GetTeamIndex() != teamIndex)
		{
			costMap->SetHeat(currActor->GetMapCoordinate().xy(), 99999999.f);
		}
	}

	// Actual distance map, used for path reconstruction
	out_distanceMap = new HeatMap(m_map->GetMapDimensions2D(), 9999999.f);
	out_distanceMap->Seed(0.f, startBlock.xy());

	// Keep iterating until we no longer notice a change in the map
	bool valueChanged = true;
	while (valueChanged)
	{
		valueChanged = false;

		// Iterate across the map
		for (int xIndex = 0; xIndex < m_map->GetMapWidth(); xIndex++)
		{
			for (int yIndex = 0; yIndex < m_map->GetMapDepth(); yIndex++)
			{
				int currIndex = out_distanceMap->GetIndex(xIndex, yIndex);

				// Update the current index by checking all 4 neighbors
				int southIndex      = out_distanceMap->GetIndex(xIndex, yIndex - 1);
				bool southChanged   = UpdateCurrFromNeighbor(currIndex, southIndex, speed, jump, costMap, m_map->GetHeightMap(), out_distanceMap);

				int northIndex      = out_distanceMap->GetIndex(xIndex, yIndex + 1);
				bool northChanged   = UpdateCurrFromNeighbor(currIndex, northIndex, speed, jump, costMap, m_map->GetHeightMap(), out_distanceMap);

				int westIndex       = out_distanceMap->GetIndex(xIndex - 1, yIndex);
				bool westChanged    = UpdateCurrFromNeighbor(currIndex, westIndex, speed, jump, costMap, m_map->GetHeightMap(), out_distanceMap);

				int eastIndex       = out_distanceMap->GetIndex(xIndex + 1, yIndex);
				bool eastChanged    = UpdateCurrFromNeighbor(currIndex, eastIndex, speed, jump, costMap, m_map->GetHeightMap(), out_distanceMap);

				// Update if we changed or not
				valueChanged = valueChanged || northChanged || southChanged || eastChanged || westChanged;
			}
		}
	}

	// Assemble the vector
	for (int x = 0; x < m_map->GetMapWidth(); x++)
	{
		for (int y = 0; y < m_map->GetMapDepth(); y++)
		{
			// Prevent choosing the same tile a friendly actor is already on (enemy actors already factored into the pathing)
			Actor* currActor = GetActorAtCoords(IntVector2(x, y));
			if (currActor != nullptr && currActor->GetTeamIndex() == teamIndex && (IntVector2(x,y) != startBlock.xy()))
			{
				continue;
			}

			float currDist = out_distanceMap->GetHeat(IntVector2(x, y));
			if (currDist <= speed)
			{
				coords.push_back(IntVector3(x, y, m_map->GetHeightAtMapCoords(IntVector2(x, y))));
			}
		}
	}
	return coords;
}


std::vector<IntVector3> BoardState::GetAttackableCoords(const IntVector3& startBlock) const
{
	std::vector<IntVector3> attackableCoords;

	// I'm not going to let actors attack themselves
	IntVector3 northBlock	= m_map->GetNeighborCoords(startBlock, IntVector2::STEP_NORTH);
	IntVector3 southBlock	= m_map->GetNeighborCoords(startBlock, IntVector2::STEP_SOUTH);
	IntVector3 eastBlock	= m_map->GetNeighborCoords(startBlock, IntVector2::STEP_EAST);
	IntVector3 westBlock	= m_map->GetNeighborCoords(startBlock, IntVector2::STEP_WEST);
	
	if (m_map->Are3DCoordsValid(northBlock))	{ attackableCoords.push_back(northBlock); }
	if (m_map->Are3DCoordsValid(southBlock))	{ attackableCoords.push_back(southBlock); }
	if (m_map->Are3DCoordsValid(eastBlock))	{ attackableCoords.push_back(eastBlock);  }
	if (m_map->Are3DCoordsValid(westBlock))	{ attackableCoords.push_back(westBlock);  }

	return attackableCoords;
}


//-----------------------------------------------------------------------------------------------
// Returns all coords that are within the xz Manhattan distance to start block
//
std::vector<IntVector3> BoardState::GetCoordsWithinManhattanDistance(const IntVector3& startBlock, int minDistance, int maxDistance) const
{
	std::vector<IntVector3> coords;

	int mapWidth = m_map->GetMapWidth();
	int mapDepth = m_map->GetMapDepth();

	for (int currX = 0; currX < mapWidth; ++currX)
	{
		for (int currY = 0; currY < mapDepth; ++currY)
		{
			int xDistance = AbsoluteValue((startBlock.x - currX));
			int yDistance = AbsoluteValue((startBlock.y - currY));

			int totalDistance = xDistance + yDistance;

			if (totalDistance >= minDistance && totalDistance <= maxDistance)
			{
				coords.push_back(IntVector3(currX, currY, m_map->GetHeightAtMapCoords(IntVector2(currX, currY))));
			}
		}
	}

	return coords;
}


//-----------------------------------------------------------------------------------------------
// Returns the map direction vector corresponding to which is up relative to the camera's view
//
IntVector2 BoardState::GetMapUpRelativeToCamera() const
{
	// Get the game camera's forward vector
	OrbitCamera* camera = Game::GetGameCamera();
	Vector3 cameraForward = camera->GetForwardVector();

	// Find the max dot product
	float northDot = DotProduct(Vector3(0.f, 0.f, 1.f), cameraForward);
	float southDot = DotProduct(Vector3(0.f, 0.f, -1.f), cameraForward);
	float eastDot  = DotProduct(Vector3(1.f, 0.f, 0.f), cameraForward);
	float westDot  = DotProduct(Vector3(-1.f, 0.f, 0.f), cameraForward);

	float maxDot = MaxFloat(northDot, southDot, eastDot, westDot);

	// Return the map vector corresponding to what is "up"
	IntVector2 mapUpRelativeToCamera;

	if		(maxDot == northDot)	{ mapUpRelativeToCamera = IntVector2::STEP_NORTH; }
	else if (maxDot == southDot)	{ mapUpRelativeToCamera = IntVector2::STEP_SOUTH; }
	else if (maxDot == eastDot)		{ mapUpRelativeToCamera = IntVector2::STEP_EAST; }
	else							{ mapUpRelativeToCamera = IntVector2::STEP_WEST; }

	return mapUpRelativeToCamera;
}


//-----------------------------------------------------------------------------------------------
// Adds the given actor to the list of actors - Does NOT check for duplicates
//
void BoardState::AddActor(const std::string& actorName, ActorDefinition* definition, const IntVector2& spawnCoords, unsigned int teamIndex)
{
	m_actors.push_back(new Actor(actorName, definition, spawnCoords, teamIndex));
	m_actors.back()->SetTurnCount((int)m_actors.size() + 999);
}


//-----------------------------------------------------------------------------------------------
// Deletes all actors from the encounter - assumes there are no duplicates
//
void BoardState::DestroyAllActors()
{
	std::vector<Actor*>::iterator itr = m_actors.begin();
	for (itr; itr != m_actors.end(); itr++)
	{
		delete *itr;
	}
	m_actors.clear();
}


//-----------------------------------------------------------------------------------------------
// Returns the number of actors in this encounter
//
unsigned int BoardState::GetActorCount()
{
	return (unsigned int) m_actors.size();
}


//-----------------------------------------------------------------------------------------------
// Returns the actor at the index actorIndex
//
Actor* BoardState::GetActorByIndex(unsigned int actorIndex) const
{
	if (actorIndex >= m_actors.size())
	{
		return nullptr;
	}

	return m_actors[actorIndex];
}


//-----------------------------------------------------------------------------------------------
// Returns the actor at the given mapCoords if one is present there, or nullptr if no actor is there
//
Actor* BoardState::GetActorAtCoords(const IntVector2& mapCoords) const
{
	std::vector<Actor*>::const_iterator itr = m_actors.begin();
	for (itr; itr != m_actors.end(); itr++)
	{
		Actor* currActor = *itr;
		if (currActor->GetMapCoordinate().xy() == mapCoords)
		{
			return currActor;
		}
	}

	// Return null if no actor exists at that position
	return nullptr;
}


//-----------------------------------------------------------------------------------------------
// Deletes the given actor from the encounter
//
void BoardState::DeleteActor(Actor* actor)
{
	for (int actorIndex = (int) m_actors.size() - 1; actorIndex >= 0; --actorIndex)
	{
		if (m_actors[actorIndex] == actor)
		{
			delete m_actors[actorIndex];
			m_actors.erase(m_actors.begin() + actorIndex);
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Spawns all actors from this BoardState's definition
//
void BoardState::SpawnActorsFromDefinition()
{
	std::vector<ActorSpawnInfo> spawnInfo = m_definition->GetActorSpawnInfo();

	for (int spawnIndex = 0; spawnIndex < (int) spawnInfo.size(); ++spawnIndex)
	{
		ActorSpawnInfo currSpawnInfo = spawnInfo[spawnIndex];
		AddActor(currSpawnInfo.m_name, currSpawnInfo.m_definition, currSpawnInfo.m_spawnPosition, currSpawnInfo.m_teamIndex);
	}
}


//-----------------------------------------------------------------------------------------------
// Calls Actor::Update() on all actors
//
void BoardState::UpdateAllActors()
{
	std::vector<Actor*>::const_iterator itr = m_actors.begin();
	for (itr; itr != m_actors.end(); itr++)
	{
		Actor* currActor = *itr;
		if (!currActor->IsDead())
		{
			currActor->Update();
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Checks the actor's list for dead actors and deletes them
//
void BoardState::DeleteDeadActors()
{
	for (int actorIndex = (int) m_actors.size() - 1; actorIndex >= 0; --actorIndex)
	{
		if (m_actors[actorIndex]->IsDead())
		{
			delete m_actors[actorIndex];
			m_actors.erase(m_actors.begin() + actorIndex);
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Decrements all actors' wait times by (numDecrementTicks * m_actionSpeed)
//
void BoardState::DecrementAllActorWaitTimes(float numDecrementTicks)
{
	for (int actorIndex = (int) m_actors.size() - 1; actorIndex >= 0; --actorIndex)
	{
		Actor* currActor = m_actors[actorIndex];
		currActor->AddToWaitTime(-1.0f * numDecrementTicks * currActor->GetActionSpeed());
	}
}


//-----------------------------------------------------------------------------------------------
// Sorts the actors so that they are ordered to when they will act next
//
void BoardState::SortActorsByNextTurn()
{
	for (int insertIndex = 0; insertIndex < (int) m_actors.size() - 1; ++insertIndex)
	{
		int		minIndex = -1;
		float	minTime = 9999.f;
		int		minTurns = -1;

		for (int checkIndex = insertIndex; checkIndex < (int) m_actors.size(); ++checkIndex)
		{
			float	currTime = m_actors[checkIndex]->GetTimeUntilNextAction();
			int		currTurns = m_actors[checkIndex]->GetTurnsSinceLastAction();

			if (currTime < minTime || (currTime == minTime && currTurns > minTurns))
			{
				minIndex = checkIndex;
				minTime = currTime;
				minTurns = currTurns;
			}
		}


		// Place the min actor at the insert index
		Actor* temp = m_actors[insertIndex];
		m_actors[insertIndex] = m_actors[minIndex];
		m_actors[minIndex] = temp;
	}

	// Sanity check here
	for (int index = 0; index < (int) m_actors.size() - 1; ++index)
	{
		Actor* currActor = m_actors[index];
		Actor* nextActor = m_actors[index + 1];

		bool hasLessWaitTime = currActor->GetTimeUntilNextAction() < nextActor->GetTimeUntilNextAction();
		bool hasEqualTimeButMoreTurns = (currActor->GetTimeUntilNextAction() == nextActor->GetTimeUntilNextAction()) && currActor->GetTurnsSinceLastAction() > nextActor->GetTurnsSinceLastAction();
		ASSERT_OR_DIE(hasLessWaitTime || hasEqualTimeButMoreTurns, "It didn't work");
	}
}


//-----------------------------------------------------------------------------------------------
// Returns the actor's order for next move in the actor list
//
int BoardState::GetActorTurnOrder(Actor* actor) const
{
	for (int index = 0; index < (int) m_actors.size(); ++index)
	{
		if (m_actors[index] == actor)
		{
			return index;
		}
	}

	return -1;
}


//-----------------------------------------------------------------------------------------------
// Draws the actors' sprites
//
void BoardState::RenderActors() const
{
	Renderer* renderer = Renderer::GetInstance();
	OrbitCamera* gameCamera = Game::GetGameCamera();
	renderer->SetCurrentCamera(gameCamera);

	// Render all the sprites first
	std::vector<Actor*>::const_iterator itr = m_actors.begin();
	for (itr; itr != m_actors.end(); itr++)
	{
		Actor* currActor = *itr;
		currActor->RenderSprite();
	}

	// Then render all the healthbars, ensures healthbars are on top
	itr = m_actors.begin();
	for (itr; itr != m_actors.end(); itr++)
	{
		Actor* currActor = *itr;
		currActor->RenderHealthBar();
	}
}


//-----------------------------------------------------------------------------------------------
// Constructs the map from the boardstate's definition
//
void BoardState::InitializeMapFromDefinition()
{
	m_map = new Map();
	m_map->LoadFromHeightMap(m_definition->GetMapFilePath().c_str());
}
