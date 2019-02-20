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
#include "Engine/Core/File.hpp"
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
	// Don't use DeactivateChunk() to avoid invalidating this iterator
	std::map<IntVector2, Chunk*>::iterator chunkItr = m_activeChunks.begin();

	for (chunkItr; chunkItr != m_activeChunks.end(); chunkItr++)
	{
		Chunk* chunk = chunkItr->second;

		if (chunk->ShouldWriteToFile())
		{
			chunk->WriteToFile();
		}

		delete chunk;
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
	CheckToBuildChunkMesh();
	CheckToDeactivateChunks();

	UpdateChunks();
}


//-----------------------------------------------------------------------------------------------
// Renders all chunks to the screen
//
void World::Render() const
{
	RenderChunks();
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
// Does not check the Z bounds
//
IntVector2 World::GetChunkCoordsForChunkThatContainsPosition(const Vector3& position) const
{
	return GetChunkCoordsForChunkThatContainsPosition(position.xy());
}


//-----------------------------------------------------------------------------------------------
// Returns the chunk that contains the given position
//
Chunk* World::GetChunkThatContainsPosition(const Vector2& position) const
{
	IntVector2 chunkCoords = GetChunkCoordsForChunkThatContainsPosition(position);

	bool chunkExists = m_activeChunks.find(chunkCoords) != m_activeChunks.end();
	if (chunkExists)
	{
		return m_activeChunks.at(chunkCoords);
	}

	return nullptr;
}


//-----------------------------------------------------------------------------------------------
// Returns the chunk that contains the given position
// Does not check the Z bounds
//
Chunk* World::GetChunkThatContainsPosition(const Vector3& position) const
{
	return GetChunkThatContainsPosition(position.xy());
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

	// If the file for a chunk exists, load it
	std::string filename = Stringf("Data/Saves/Chunk_%i_%i.chunk", chunkCoords.x, chunkCoords.y);
	bool fromFileSuccess = chunk->InitializeFromFile(filename);

	if (fromFileSuccess)
	{
		ConsolePrintf(Rgba::GREEN, "Chunk (%i, %i) activated from file", chunkCoords.x, chunkCoords.y);
	}
	else
	{
		chunk->GenerateWithPerlinNoise(BASE_ELEVATION, NOISE_MAX_DEVIATION_FROM_BASE_ELEVATION);
		ConsolePrintf(Rgba::GREEN, "Chunk (%i, %i) gererated from noise", chunkCoords.x, chunkCoords.y);
	}

	// Add the chunk to the active list; its mesh will be built later
	AddChunkToActiveList(chunk);
}


//-----------------------------------------------------------------------------------------------
// Removes the chunk from the world's list of active chunks, and writes to file if it has been modified
// DELETES THE CHUNK
//
void World::DeactivateChunk(Chunk* chunk)
{
	RemoveChunkFromActiveList(chunk);

	// Write to file if needed
	if (chunk->ShouldWriteToFile())
	{
		chunk->WriteToFile();
	}

	delete chunk;
}


//-----------------------------------------------------------------------------------------------
// Checks if there is an inactive chunk within the activation range, and returns true if there exists
// one, returning the closest one
//
bool World::GetClosestInactiveChunkCoordsToPlayerWithinActivationRange(IntVector2& out_closestInactiveChunkCoords) const
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
Chunk* World::GetFarthestActiveChunkToPlayerOutsideDeactivationRange() const
{
	std::map<IntVector2, Chunk*>::const_iterator itr = m_activeChunks.begin();

	Vector2 cameraXYPosition = Game::GetGameCamera()->GetPosition().xy();

	float deactivationRangeSquared = CHUNK_DEACTIVATION_RANGE * CHUNK_DEACTIVATION_RANGE;
	float maxDistance = 0.f;
	Chunk* farthestActiveChunkOutsideDeactivationRange = nullptr;
	
	for (itr; itr != m_activeChunks.end(); itr++)
	{
		Chunk* currChunk = itr->second;

		Vector2 chunkXYCenter = currChunk->GetWorldXYCenter();
		Vector2 vectorToChunk = (chunkXYCenter - cameraXYPosition);
		float distanceSquared = vectorToChunk.GetLengthSquared();

		if (distanceSquared > deactivationRangeSquared && distanceSquared > maxDistance)
		{
			maxDistance = distanceSquared;
			farthestActiveChunkOutsideDeactivationRange = currChunk;
		}
	}

	return farthestActiveChunkOutsideDeactivationRange;
}


//-----------------------------------------------------------------------------------------------
// Checks for a chunk that needs its mesh rebuilt, and rebuilds it if found
//
void World::CheckToBuildChunkMesh()
{
	IntVector2 closestDirtyCoords;
	bool dirtyMeshFound = GetClosestActiveChunkToPlayerWithDirtyMesh(closestDirtyCoords);

	if (dirtyMeshFound)
	{
		Chunk* dirtyMeshChunk = m_activeChunks[closestDirtyCoords];
		dirtyMeshChunk->BuildMesh();
	}
}


//-----------------------------------------------------------------------------------------------
// Searches for a chunk in the active list that has a dirty mesh, and returns the closest one
//
bool World::GetClosestActiveChunkToPlayerWithDirtyMesh(IntVector2& out_closestActiveDirtyCoords) const
{
	std::map<IntVector2, Chunk*>::const_iterator itr = m_activeChunks.begin();

	Vector2 cameraXYPosition = Game::GetGameCamera()->GetPosition().xy();

	float minDistance = 0.f;
	bool foundDirtyChunk = false;

	for (itr; itr != m_activeChunks.end(); itr++)
	{
		Chunk* currChunk = itr->second;

		if (!currChunk->IsMeshDirty() || !currChunk->HasAllFourNeighbors())
		{
			continue;
		}

		// Get the distance to the dirty chunk
		Vector2 chunkXYCenter = currChunk->GetWorldXYCenter();

		Vector2 vectorToChunk = (chunkXYCenter - cameraXYPosition);
		float distanceSquared = vectorToChunk.GetLengthSquared();

		if (!foundDirtyChunk || distanceSquared < minDistance)
		{
			minDistance = distanceSquared;
			out_closestActiveDirtyCoords = currChunk->GetChunkCoords();
			foundDirtyChunk = true;
		}
	}

	return foundDirtyChunk;
}


//-----------------------------------------------------------------------------------------------
// Calls Update() on all chunks
//
void World::UpdateChunks()
{
	std::map<IntVector2, Chunk*>::iterator chunkItr = m_activeChunks.begin();

	for (chunkItr; chunkItr != m_activeChunks.end(); chunkItr++)
	{
		chunkItr->second->Update();
	}
}


//-----------------------------------------------------------------------------------------------
// Calls Render() on all the chunk
//
void World::RenderChunks() const
{
	std::map<IntVector2, Chunk*>::const_iterator chunkItr = m_activeChunks.begin();

	for (chunkItr; chunkItr != m_activeChunks.end(); chunkItr++)
	{
		chunkItr->second->Render();
	}
}


//-----------------------------------------------------------------------------------------------
// Adds the chunk to the world map, and connects any references the chunk has to its neighbors
//
void World::AddChunkToActiveList(Chunk* chunkToAdd)
{
	// Check for duplicates
	IntVector2 chunkCoords = chunkToAdd->GetChunkCoords();
	bool alreadyExists = m_activeChunks.find(chunkCoords) != m_activeChunks.end();
	ASSERT_OR_DIE(!alreadyExists, Stringf("World attempted to add duplicate chunk at coords (%i, %i)", chunkCoords.x, chunkCoords.y).c_str());

	// Add it to the map
	m_activeChunks[chunkCoords] = chunkToAdd;


	// Hook up the references to the neighbors
	IntVector2 eastCoords = chunkCoords + IntVector2(1, 0);
	IntVector2 westCoords = chunkCoords + IntVector2(-1, 0);
	IntVector2 northCoords = chunkCoords + IntVector2(0, 1);
	IntVector2 southCoords = chunkCoords + IntVector2(0, -1);

	bool eastExists = m_activeChunks.find(eastCoords) != m_activeChunks.end();
	bool westExists = m_activeChunks.find(westCoords) != m_activeChunks.end();
	bool northExists = m_activeChunks.find(northCoords) != m_activeChunks.end();
	bool southExists = m_activeChunks.find(southCoords) != m_activeChunks.end();

	if (eastExists)
	{
		Chunk* eastChunk = m_activeChunks[eastCoords];

		chunkToAdd->SetEastNeighbor(eastChunk);
		eastChunk->SetWestNeighbor(chunkToAdd);
	}

	if (westExists)
	{
		Chunk* westChunk = m_activeChunks[westCoords];

		chunkToAdd->SetWestNeighbor(westChunk);
		westChunk->SetEastNeighbor(chunkToAdd);
	}

	if (northExists)
	{
		Chunk* northChunk = m_activeChunks[northCoords];

		chunkToAdd->SetNorthNeighbor(northChunk);
		northChunk->SetSouthNeighbor(chunkToAdd);
	}

	if (southExists)
	{
		Chunk* southChunk = m_activeChunks[southCoords];

		chunkToAdd->SetSouthNeighbor(southChunk);
		southChunk->SetNorthNeighbor(chunkToAdd);
	}
}


//-----------------------------------------------------------------------------------------------
// Removes this chunk from the active list in the world and unhooks any references to neighbors
//
void World::RemoveChunkFromActiveList(Chunk* chunkToRemove)
{
	IntVector2 chunkCoords = chunkToRemove->GetChunkCoords();
	bool chunkExists = m_activeChunks.find(chunkCoords) != m_activeChunks.end();
	ASSERT_OR_DIE(chunkExists, "World tried to remove chunk that doesn't exist");

	m_activeChunks.erase(chunkCoords);

	// Remove the connections
	Chunk* eastNeighbor = chunkToRemove->GetEastNeighbor();
	Chunk* westNeighbor = chunkToRemove->GetWestNeighbor();
	Chunk* northNeighbor = chunkToRemove->GetNorthNeighbor();
	Chunk* southNeighbor = chunkToRemove->GetSouthNeighbor();

	if (eastNeighbor != nullptr)
	{
		eastNeighbor->SetWestNeighbor(nullptr);
	}

	if (westNeighbor != nullptr)
	{
		westNeighbor->SetEastNeighbor(nullptr);
	}

	if (northNeighbor != nullptr)
	{
		northNeighbor->SetSouthNeighbor(nullptr);
	}

	if (southNeighbor != nullptr)
	{
		southNeighbor->SetNorthNeighbor(nullptr);
	}

	chunkToRemove->SetEastNeighbor(nullptr);
	chunkToRemove->SetWestNeighbor(nullptr);
	chunkToRemove->SetNorthNeighbor(nullptr);
	chunkToRemove->SetSouthNeighbor(nullptr);
}


//-----------------------------------------------------------------------------------------------
// Checks if there is a chunk within the activation range that is inactive, and activates it
//
void World::CheckToActivateChunks()
{
	IntVector2 closestInactiveChunkCoords;
	bool foundInactiveChunk = GetClosestInactiveChunkCoordsToPlayerWithinActivationRange(closestInactiveChunkCoords);

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
	Chunk* chunkToDeactivate = GetFarthestActiveChunkToPlayerOutsideDeactivationRange();

	if (chunkToDeactivate != nullptr)
	{
		IntVector2 chunkCoords = chunkToDeactivate->GetChunkCoords();
		ConsolePrintf(Rgba::ORANGE, "Deactivating Chunk (%i, %i)", chunkCoords.x, chunkCoords.y);
		DeactivateChunk(chunkToDeactivate);
	}
}
