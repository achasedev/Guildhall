/************************************************************************/
/* File: World.hpp
/* Author: Andrew Chase
/* Date: February 9th 2019
/* Description: Class to represent a game "world" of chunks
/************************************************************************/
#pragma once
#include <map>
#include "Engine/Math/IntVector2.hpp"
#include "Game/Environment/BlockLocator.hpp"

// For raycasting into blocks in the world
struct RaycastResult_t
{
	Vector3			m_startPosition = Vector3::ZERO;
	Vector3			m_direction = Vector3::ZERO;
	float			m_maxDistance = 0.f;
	Vector3			m_endPosition = Vector3::ZERO;
	Vector3			m_impactPosition = Vector3::ZERO;
	float			m_impactFraction = 0.f;
	float			m_impactDistance = 0.f;
	BlockLocator	m_impactBlock = BlockLocator(nullptr, 0);
	Vector3			m_impactNormal = Vector3::ZERO;

	bool DidImpact() const { return m_impactFraction < 1.f; }
};

class Chunk;

class World
{
public:
	//-----Public Methods-----

	World();
	~World();

	void ProcessInput();
	void Update();
	void Render() const;

	IntVector2		GetChunkCoordsForChunkThatContainsPosition(const Vector2& position) const;
	IntVector2		GetChunkCoordsForChunkThatContainsPosition(const Vector3& position) const;
	Chunk*			GetChunkThatContainsPosition(const Vector2& position) const;
	Chunk*			GetChunkThatContainsPosition(const Vector3& position) const;
	BlockLocator	GetBlockLocatorThatContainsPosition(const Vector3& position) const;

	RaycastResult_t Raycast(const Vector3& start, const Vector3& directionNormal, float maxDistance) const;


private:
	//-----Private Methods-----

	void AddChunkToActiveList(Chunk* chunkToAdd);
	void RemoveChunkFromActiveList(Chunk* chunkToRemove);

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
	void UpdateRaycast();

	// Render
	void RenderChunks() const;


private:
	//-----Private Data-----
	
	std::map<IntVector2, Chunk*> m_activeChunks;

	// For Debugging
	bool m_raycastDetached = false;
	Vector3 m_raycastReferencePosition; // Will be camera position when not detached
	Vector3 m_raycastForward;			// Will be the camera forward when not detached
	RaycastResult_t m_lastRaycastResult;
	uint8_t m_blockTypeToPlace = 5;

	static constexpr int SEA_LEVEL = 20;
	static constexpr int BASE_ELEVATION = 30;
	static constexpr int NOISE_MAX_DEVIATION_FROM_BASE_ELEVATION = 10;
	static constexpr int RAYCAST_STEPS_PER_BLOCK = 100;
	static constexpr float DEFAULT_RAYCAST_DISTANCE = 8.f;

};
