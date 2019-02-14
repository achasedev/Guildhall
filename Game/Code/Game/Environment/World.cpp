/************************************************************************/
/* File: World.cpp
/* Author: Andrew Chase
/* Date: February 9th 2019
/* Description: Implementation of the World class
/************************************************************************/
#include "Game/Framework/Game.hpp"
#include "Game/Environment/World.hpp"
#include "Game/Environment/Chunk.hpp"
#include "Game/Framework/GameCamera.hpp"
#include "Game/Framework/GameCommon.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Utility/StringUtils.hpp"
#include "Engine/Core/Utility/ErrorWarningAssert.hpp"
#include "Engine/Core/DeveloperConsole/DevConsole.hpp"


//-----------------------------------------------------------------------------------------------
// Constructor
//
World::World()
{
}


//-----------------------------------------------------------------------------------------------
// Destructor
//
World::~World()
{
	// Delete all active chunks
	std::map<IntVector2, Chunk*>::iterator chunkItr = m_activeChunks.begin();

	for (chunkItr; chunkItr != m_activeChunks.end(); chunkItr++)
	{
		delete chunkItr->second;
	}

	m_activeChunks.clear();
}


//-----------------------------------------------------------------------------------------------
// Updates all chunks in the world, as well as the camera
//
void World::Update()
{
	Game::GetGameCamera()->Update();

	// Activate chunks within the range
	CheckToActivateChunks();
	CheckToDeactivateChunks();

	std::map<IntVector2, Chunk*>::iterator chunkItr = m_activeChunks.begin();

	for (chunkItr; chunkItr != m_activeChunks.end(); chunkItr++)
	{
		chunkItr->second->Update();
	}
}


//-----------------------------------------------------------------------------------------------
// Renders all chunks to the screen
//
void World::Render() const
{
	std::map<IntVector2, Chunk*>::const_iterator chunkItr = m_activeChunks.begin();

	for (chunkItr; chunkItr != m_activeChunks.end(); chunkItr++)
	{
		chunkItr->second->Render();
	}
}


//-----------------------------------------------------------------------------------------------
// Returns the chunk coordinates for the chunk that contains the given position
//
IntVector2 World::GetChunkCoordsForChunkThatContainsPosition(const Vector2& position) const
{
	int x = Floor(position.x / (float)Chunk::CHUNK_DIMENSIONS_X);
	int y = Floor(position.y / (float)Chunk::CHUNK_DIMENSIONS_Y);

	return IntVector2(x, y);
}


//-----------------------------------------------------------------------------------------------
// Returns the chunk coordinates that contain the given position
//
IntVector2 World::GetChunkCoordsForChunkThatContainsPosition(const Vector3& position) const
{
	return GetChunkCoordsForChunkThatContainsPosition(position.xy());
}


//-----------------------------------------------------------------------------------------------
// Loads or generates a chunk at the given coords and adds it to the world
//
void World::ActivateChunk(const IntVector2& chunkCoords)
{
	// Ensure we don't try to activate an already active chunk
	bool alreadyActive = m_activeChunks.find(chunkCoords) != m_activeChunks.end();
	ASSERT_OR_DIE(!alreadyActive, Stringf("World tried to activate chunk at coords (%i, %i) but it was already active", chunkCoords.x, chunkCoords.y).c_str());

	// Generate with Perlin noise for now
	Chunk* chunk = new Chunk(chunkCoords);
	chunk->GenerateWithPerlinNoise(BASE_ELEVATION, NOISE_MAX_DEVIATION_FROM_BASE_ELEVATION);

	m_activeChunks[chunkCoords] = chunk;
}


//-----------------------------------------------------------------------------------------------
// Removes the chunk given by the chunk coords, and writes to file if it has been modified
//
void World::DeactivateChunk(const IntVector2& chunkCoords)
{
	//  #TODO: Write to file
	bool chunkIsActive = m_activeChunks.find(chunkCoords) != m_activeChunks.end();
	ASSERT_OR_DIE(chunkIsActive, "World tried to deactivate a chunk that doesn't exist");

	Chunk* chunk = m_activeChunks[chunkCoords];
	delete chunk;
	m_activeChunks.erase(chunkCoords);
}


//-----------------------------------------------------------------------------------------------
// Checks if there is an inactive chunk within the activation range, and returns true if there exists
// one, returning the closest one
//
bool World::GetClosestInactiveChunkToPlayerWithinActivationRange(IntVector2& out_closestInactiveChunkCoords) const
{
	int chunkSpanX = Ceiling(CHUNK_ACTIVATION_RANGE / (float)Chunk::CHUNK_DIMENSIONS_X);
	int chunkSpanY = Ceiling(CHUNK_ACTIVATION_RANGE / (float)Chunk::CHUNK_DIMENSIONS_Y);
	IntVector2 chunkSpan = IntVector2(chunkSpanX, chunkSpanY);

	Vector2 cameraXYPosition = Game::GetGameCamera()->GetPosition().xy();
	IntVector2 chunkContainingCamera = GetChunkCoordsForChunkThatContainsPosition(cameraXYPosition);

	IntVector2 startChunk = chunkContainingCamera - chunkSpan;
	IntVector2 endChunk = chunkContainingCamera + chunkSpan;

	float activationRangeSquared = CHUNK_ACTIVATION_RANGE * CHUNK_ACTIVATION_RANGE;
	float minDistanceSoFar = activationRangeSquared;
	bool foundInactiveChunk = false;

	for (int y = startChunk.y; y <= endChunk.y; ++y)
	{
		for (int x = startChunk.x; x <= endChunk.x; ++x)
		{
			IntVector2 currChunkCoords = IntVector2(x, y);

			// If the chunk is already active just continue
			if (m_activeChunks.find(currChunkCoords) != m_activeChunks.end())
			{
				continue;
			}

			// Get the distance to the chunk
			Vector2 chunkBasePosition = Vector2(currChunkCoords.x * Chunk::CHUNK_DIMENSIONS_X, currChunkCoords.y * Chunk::CHUNK_DIMENSIONS_Y);
			Vector2 chunkXYCenter = chunkBasePosition + 0.5f * Vector2(Chunk::CHUNK_DIMENSIONS_X, Chunk::CHUNK_DIMENSIONS_Y);
			Vector2 vectorToChunkCenter = (chunkXYCenter - cameraXYPosition);

			float distanceSquared = vectorToChunkCenter.GetLengthSquared();
			
			// Update our min if it's smaller
			if (distanceSquared < minDistanceSoFar)
			{
				minDistanceSoFar = distanceSquared;
				out_closestInactiveChunkCoords = currChunkCoords;
				foundInactiveChunk = true;
			}
		}
	}

	return foundInactiveChunk;
}


//-----------------------------------------------------------------------------------------------
// Returns true if it finds an active chunk outside the deactivation range, and returns the coords
// of the closest one
//
bool World::GetFarthestActiveChunkToPlayerOutsideDeactivationRange(IntVector2& out_closestActiveChunkCoords) const
{
	std::map<IntVector2, Chunk*>::const_iterator itr = m_activeChunks.begin();

	Vector2 cameraXYPosition = Game::GetGameCamera()->GetPosition().xy();

	float deactivationRangeSquared = CHUNK_DEACTIVATION_RANGE * CHUNK_DEACTIVATION_RANGE;
	float maxDistance = 0.f;
	bool foundChunkToDeactivate = false;

	for (itr; itr != m_activeChunks.end(); itr++)
	{
		Chunk* currChunk = itr->second;

		Vector2 chunkXYCenter = currChunk->GetWorldXYCenter();
		
		Vector2 vectorToChunk = (chunkXYCenter - cameraXYPosition);
		float distanceSquared = vectorToChunk.GetLengthSquared();

		if (distanceSquared > deactivationRangeSquared && distanceSquared > maxDistance)
		{
			maxDistance = distanceSquared;
			out_closestActiveChunkCoords = currChunk->GetChunkCoords();
			foundChunkToDeactivate = true;
		}
	}

	return foundChunkToDeactivate;
}


//-----------------------------------------------------------------------------------------------
// Checks if there is a chunk within the activation range that is inactive, and activates it
//
void World::CheckToActivateChunks()
{
	IntVector2 closestInactiveChunkCoords;
	bool foundInactiveChunk = GetClosestInactiveChunkToPlayerWithinActivationRange(closestInactiveChunkCoords);

	if (foundInactiveChunk)
	{
		ConsolePrintf("Activating Chunk (%i, %i)", closestInactiveChunkCoords.x, closestInactiveChunkCoords.y);
		ActivateChunk(closestInactiveChunkCoords);
	}
}


//-----------------------------------------------------------------------------------------------
// Checks if any active chunks are outside the deactivation range, and if so removes them
//
void World::CheckToDeactivateChunks()
{
	IntVector2 closestActiveChunkCoords;
	bool foundActiveChunkToDeactivate = GetFarthestActiveChunkToPlayerOutsideDeactivationRange(closestActiveChunkCoords);

	if (foundActiveChunkToDeactivate)
	{
		ConsolePrintf(Rgba::ORANGE, "Deactivating Chunk (%i, %i)", closestActiveChunkCoords.x, closestActiveChunkCoords.y);
		DeactivateChunk(closestActiveChunkCoords);
	}
}
