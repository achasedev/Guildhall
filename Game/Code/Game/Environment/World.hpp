/************************************************************************/
/* File: World.hpp
/* Author: Andrew Chase
/* Date: February 9th 2019
/* Description: Class to represent a game "world" of chunks
/************************************************************************/
#pragma once
#include <map>
#include "Engine/Math/IntVector2.hpp"

class Chunk;

class World
{
public:
	//-----Public Methods-----

	World();
	~World();

	void Update();
	void Render() const;

	IntVector2	GetChunkCoordsForChunkThatContainsPosition(const Vector2& position) const;
	IntVector2	GetChunkCoordsForChunkThatContainsPosition(const Vector3& position) const;

	Chunk*		GetChunkThatContainsPosition(const Vector2& position) const;
	Chunk*		GetChunkThatContainsPosition(const Vector3& position) const;


private:
	//-----Private Methods-----

	void	AddChunkToActiveList(Chunk* chunkToAdd);
	void	RemoveChunkFromActiveList(Chunk* chunkToRemove);

	// Chunk Activation
	void CheckToActivateChunks();
	void ActivateChunk(const IntVector2& chunkCoords);
	bool GetClosestInactiveChunkCoordsToPlayerWithinActivationRange(IntVector2& out_closestInactiveChunkCoords) const;

	// Chunk Deactivation
	void CheckToDeactivateChunks();
	void DeactivateChunk(Chunk* chunk);
	Chunk* GetFarthestActiveChunkToPlayerOutsideDeactivationRange() const;

	// Chunk MeshBuilding
	void CheckToBuildChunkMesh();
	bool GetClosestActiveChunkToPlayerWithDirtyMesh(IntVector2& out_closestActiveDirtyCoords) const;

	// Update
	void UpdateChunks();

	// Render
	void RenderChunks() const;


private:
	//-----Private Data-----
	
	std::map<IntVector2, Chunk*> m_activeChunks;

	static constexpr int SEA_LEVEL = 20;
	static constexpr int BASE_ELEVATION = 30;
	static constexpr int NOISE_MAX_DEVIATION_FROM_BASE_ELEVATION = 10;

};
