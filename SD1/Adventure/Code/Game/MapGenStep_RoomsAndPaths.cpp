/************************************************************************/
/* File: MapGenStep_RoomsAndPaths.hpp
/* Author: Andrew Chase
/* Date: November 6th, 2017
/* Bugs: None
/* Description: (sub)class to represent a map manipulation method
/*				Adds rectangular rooms to the map and connects them with paths
/************************************************************************/
#include <vector>
#include "Game/MapGenStep_RoomsAndPaths.hpp"
#include "Game/TileDefinition.hpp"
#include "Game/Map.hpp"
#include "Engine/Core/HeatMap.hpp"
#include "Engine/Core/XmlUtilities.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/IntVector2.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"


//-----------------------------------------------------------------------------------------------
// Constructor - from an MapGenStep XMLElement
//
MapGenStep_RoomsAndPaths::MapGenStep_RoomsAndPaths(const tinyxml2::XMLElement& genStepXmlElement)
	:MapGenStep(genStepXmlElement)
{
	// Rooms data
	const XMLElement* roomsElement = genStepXmlElement.FirstChildElement("Rooms");
	if (roomsElement != nullptr)
	{
		m_floorType = TileDefinition::ParseXMLAttribute(*roomsElement, "floorType", m_floorType);
		m_wallType	= TileDefinition::ParseXMLAttribute(*roomsElement, "wallType", m_wallType);

		m_roomWidth		= ParseXmlAttribute(*roomsElement, "width", m_roomWidth);
		m_roomHeight	= ParseXmlAttribute(*roomsElement, "height", m_roomHeight);
		m_roomCount		= ParseXmlAttribute(*roomsElement, "roomCount", m_roomCount);
		m_numOverlaps	= ParseXmlAttribute(*roomsElement, "numOverlaps", m_numOverlaps);
	}

	// Paths data
	const XMLElement* pathsElement = genStepXmlElement.FirstChildElement("Paths");
	if (pathsElement != nullptr)
	{
		m_pathType		= TileDefinition::ParseXMLAttribute(*pathsElement, "pathType", m_pathType);
		m_loopPath		= ParseXmlAttribute(*pathsElement, "loopPath", m_loopPath);
	}

	
	// Data Integrity Checks - ensure TileTypes are specified
	GUARANTEE_OR_DIE(m_floorType != nullptr, Stringf("Error: RoomsAndPaths constructor has no floorType specified."));
	GUARANTEE_OR_DIE(m_wallType != nullptr, Stringf("Error: RoomsAndPaths constructor has no wallType specified."));
	GUARANTEE_OR_DIE(m_pathType != nullptr, Stringf("Error: RoomsAndPaths constructor has no pathType specified."));
}


//-----------------------------------------------------------------------------------------------
// Destructor - unused
//
MapGenStep_RoomsAndPaths::~MapGenStep_RoomsAndPaths()
{
}


//-----------------------------------------------------------------------------------------------
// Runs the RoomsAndPaths steps given the number of iterations and chance to run
//
void MapGenStep_RoomsAndPaths::Run(Map& mapToAugment)
{
	bool shouldRunStep = CheckRandomChance(m_chanceToRun);

	if (shouldRunStep)
	{
		int numIterations = m_iterationsRange.GetRandomInRange();

		for (int i = 0; i < numIterations; i++)
		{
			RunRoomsAndPaths(mapToAugment);
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Runs the RoomsAndPaths Algorithm once, changing the map
//
void MapGenStep_RoomsAndPaths::RunRoomsAndPaths(Map& mapToAugment) const
{
	// Find all the possible room locations we can
	std::vector<AABB2> roomBoundsFound;
	FindRoomsInMap(roomBoundsFound, mapToAugment);

	// Add them to the map through changing tile definitions
	AddRoomsAndPathsToMap(roomBoundsFound, mapToAugment);
}


//-----------------------------------------------------------------------------------------------
// Finds the possible random room positions in the map and returns them in foundRooms
//
void MapGenStep_RoomsAndPaths::FindRoomsInMap(std::vector<AABB2>& out_foundRooms, Map& mapToAugment) const
{
	// Generation data
	int numFailedAttempts = 0;
	int numRoomsToMake = m_roomCount.GetRandomInRange();
	int numOverlapsRemaining = m_numOverlaps.GetRandomInRange();
	int numRoomsFound = 0;

	while (numRoomsFound < numRoomsToMake && numFailedAttempts < 1000)	// Give up after 1000 tries
	{
		int interiorWidth = m_roomWidth.GetRandomInRange();
		int interiorHeight = m_roomHeight.GetRandomInRange();

		AABB2 interiorBounds = GetRandomRoomBounds(interiorWidth, interiorHeight, mapToAugment.GetDimensions());

		// Check for overlap
		if (!DoesRoomOverlapExistingRooms(interiorBounds, out_foundRooms))
		{
			// No overlap, add it to the list and continue
			out_foundRooms.push_back(interiorBounds);
			numRoomsFound++;
		}
		else
		{
			// Room overlapped existing rooms, see if we have overlap allowances still
			if (numOverlapsRemaining > 0)
			{
				// Use an allowance and add the room
				out_foundRooms.push_back(interiorBounds);
				numRoomsFound++;
				numOverlapsRemaining--;
			}
			else
			{
				// Failed attempt with no allowances, so count the failure
				numFailedAttempts++;
			}
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Adds the room and paths to the map by changing the TileDefinitions on the Tiles
//
void MapGenStep_RoomsAndPaths::AddRoomsAndPathsToMap(std::vector<AABB2>& foundRooms, Map& mapToAugment) const
{
	AddWallsToMap(foundRooms, mapToAugment);	// Add the walls first
	AddPathsToMap(foundRooms, mapToAugment);	// Then paths
	AddFloorsToMap(foundRooms, mapToAugment);	// Then add floors
}


//-----------------------------------------------------------------------------------------------
// Adds the walls to the map given the room bounds
// Returns a distance map with all wall tiles intialized to very high numbers, for the paths step
//
HeatMap* MapGenStep_RoomsAndPaths::AddWallsToMap(std::vector<AABB2>& foundRooms, Map& mapToAugment) const
{
	int numRooms = static_cast<int>(foundRooms.size());
	HeatMap* distanceMap = new HeatMap(mapToAugment.GetDimensions(), 50.f);

	for (int roomNumber = 0; roomNumber < numRooms; roomNumber++)
	{
		AABB2 currRoomBounds = foundRooms[roomNumber];

		IntVector2 wallBottomLeftCoord	= IntVector2(currRoomBounds.mins.x, currRoomBounds.mins.y) - IntVector2(1, 1);	// Go one step back for the wall
		IntVector2 wallTopRightCoord	= IntVector2(currRoomBounds.maxs.x, currRoomBounds.maxs.y);	// Already added (1,1) before to indicate top-right corner of interior

		// Make the walls, setting the wall portions of the distance map to large values to prevent paths from running over them
		for (int xCoord = wallBottomLeftCoord.x; xCoord <= wallTopRightCoord.x; xCoord++)
		{
			IntVector2 northWall = IntVector2(xCoord, wallTopRightCoord.y);
			IntVector2 southWall = IntVector2(xCoord, wallBottomLeftCoord.y);

			mapToAugment.GetTileFromCoords(northWall)->SetDefinition(m_wallType);
			mapToAugment.GetTileFromCoords(southWall)->SetDefinition(m_wallType);

			distanceMap->SetHeat(northWall, 9999.f);
			distanceMap->SetHeat(southWall, 9999.f);
		}

		for (int yCoord = wallBottomLeftCoord.y; yCoord <= wallTopRightCoord.y; yCoord++)
		{
			IntVector2 leftWallCoord = IntVector2(wallBottomLeftCoord.x, yCoord);
			IntVector2 rightWallCoord = IntVector2(wallTopRightCoord.x, yCoord);

			mapToAugment.GetTileFromCoords(leftWallCoord)->SetDefinition(m_wallType);			
			mapToAugment.GetTileFromCoords(rightWallCoord)->SetDefinition(m_wallType);

			distanceMap->SetHeat(leftWallCoord, 9999.f);
			distanceMap->SetHeat(leftWallCoord, 9999.f);
		}
	}

	return distanceMap;
}


//-----------------------------------------------------------------------------------------------
// Adds the paths to the map given the room bounds
//
void MapGenStep_RoomsAndPaths::AddPathsToMap(std::vector<AABB2>& foundRooms, Map& mapToAugment) const
{
	int numRooms = static_cast<int>(foundRooms.size());

	for (int roomNumber = 0; roomNumber < numRooms; roomNumber++)
	{
		AABB2 firstRoom = foundRooms[roomNumber];
		AABB2 secondRoom;
		if (roomNumber == numRooms - 1)
		{
			if (m_loopPath) { secondRoom = foundRooms[0]; } // At the end, so loop path if specified
			else { break; }									// Otherwise we're done, so break
		}
		else
		{
			secondRoom = foundRooms[roomNumber + 1];
		}

		// Get the centers
		Vector2 firstRoomCenter		= firstRoom.GetCenter();
		Vector2 secondRoomCenter	= secondRoom.GetCenter();

		// Convert the positions to the tile coordinates
		IntVector2 pathStartCoord	= IntVector2(firstRoomCenter.x, firstRoomCenter.y);
		IntVector2 pathEndCoord		= IntVector2(secondRoomCenter.x, secondRoomCenter.y);

		// Generate a Dijkstra map for the paths, preserving the original map
		HeatMap* tempDistanceMap = HeatMap::ConstructDijkstraMap(mapToAugment.GetDimensions(), pathEndCoord);

		std::vector<IntVector2> path;
		tempDistanceMap->GetGreedyShortestPath(pathStartCoord, pathEndCoord, path);

		for (int pathIndex = 0; pathIndex < static_cast<int>(path.size()); pathIndex++)
		{
			IntVector2 currPathCoord = path[pathIndex];
			mapToAugment.GetTileFromCoords(currPathCoord)->SetDefinition(m_pathType);
		}

		// Free the temp distance map
		delete tempDistanceMap;
	}
}


//-----------------------------------------------------------------------------------------------
// Adds the floors to the map given the room bounds
//
void MapGenStep_RoomsAndPaths::AddFloorsToMap(std::vector<AABB2>& foundRooms, Map& mapToAugment) const
{
	int numRooms = static_cast<int>(foundRooms.size());

	for (int roomNumber = 0; roomNumber < numRooms; roomNumber++)
	{
		AABB2 roomInteriorBounds = foundRooms[roomNumber];

		IntVector2 interiorBottomLeftCoord		= IntVector2(roomInteriorBounds.mins.x, roomInteriorBounds.mins.y);
		IntVector2 interiorTopRightCoord		= IntVector2(roomInteriorBounds.maxs.x, roomInteriorBounds.maxs.y) - IntVector2(1, 1);	// Step in one to correspond to the correct tile

		for (int xCoord = interiorBottomLeftCoord.x; xCoord <= interiorTopRightCoord.x; xCoord++)
		{
			for (int yCoord = interiorBottomLeftCoord.y; yCoord <= interiorTopRightCoord.y; yCoord++)
			{
				IntVector2 currCoord = IntVector2(xCoord, yCoord);
				mapToAugment.GetTileFromCoords(currCoord)->SetDefinition(m_floorType);
			}
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Returns a random room bounds that fits in the map
//
AABB2 MapGenStep_RoomsAndPaths::GetRandomRoomBounds(int interiorWidth, int interiorHeight, const IntVector2& mapDimensions) const
{
	AABB2 possiblePositionBounds;

	possiblePositionBounds.mins = Vector2::ONES;	// Bottom-left of map, leaving one square on left and bottom for the wall
	possiblePositionBounds.maxs = Vector2(mapDimensions) - Vector2(interiorWidth, interiorHeight) - Vector2(2, 2);	// Top right compensates for 0-based indexing, space for the wall, and dimensions of the room

	Vector2 randomBottomLeft = possiblePositionBounds.GetRandomPointInside();
	Vector2 topRight = randomBottomLeft + Vector2(interiorWidth, interiorHeight) + Vector2::ONES;	// Compensate for the last row/column by stepping 1

	return AABB2(randomBottomLeft, topRight);
}


//-----------------------------------------------------------------------------------------------
// Returns true if the given room bounds overlaps the existing room bounds found for the map, 
// and false otherwise
//
bool MapGenStep_RoomsAndPaths::DoesRoomOverlapExistingRooms(const AABB2& roomBoundsToCheck, std::vector<AABB2>& foundRooms) const
{
	bool overlapsRoom = false;

	for (int roomIndex = 0; roomIndex < static_cast<int>(foundRooms.size()); roomIndex++)
	{
		overlapsRoom = overlapsRoom || DoAABBsOverlap(roomBoundsToCheck, foundRooms[roomIndex]);
	}

	return overlapsRoom;
}
