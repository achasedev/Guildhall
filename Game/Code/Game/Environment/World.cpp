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
#include "Engine/Core/Utility/StringUtils.hpp"
#include "Engine/Core/Utility/ErrorWarningAssert.hpp"


//-----------------------------------------------------------------------------------------------
// Updates all chunks in the world, as well as the camera
//
void World::Update()
{
	Game::GetGameCamera()->Update();

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
	std::map<IntVector2, Chunk*>::iterator chunkItr = m_activeChunks.begin();

	for (chunkItr; chunkItr != m_activeChunks.end(); chunkItr++)
	{
		chunkItr->second->Render();
	}
}


//-----------------------------------------------------------------------------------------------
// Loads or generates a chunk at the given coords and adds it to the world
//
void World::ActivateChunk(const IntVector2& chunkCoords)
{
	// Ensure we don't try to activate an already active chunk
	bool alreadyActive = m_activeChunks.find(chunkCoords) != m_activeChunks.end();
	ASSERT_OR_DIE(!alreadyActive, Stringf("World tried to activate chunk at coords (%i, %i) but it was already active", chunkCoords.x, chunkCoords.y).c_str());

	// Genarate with Perlin noise for now
	Chunk* chunk = new Chunk(chunkCoords);
	chunk->GenerateWithPerlinNoise(SEA_LEVEL, BASE_ELEVATION, NOISE_MAX_DEVIATION_FROM_BASE_ELEVATION);

	m_activeChunks[chunkCoords] = chunk;
}
