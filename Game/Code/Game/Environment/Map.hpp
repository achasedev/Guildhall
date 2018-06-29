/************************************************************************/
/* File: Map.hpp
/* Author: Andrew Chase
/* Date: June 3rd, 2018
/* Description: Class to represent the playable terrain
/************************************************************************/
#pragma once

#include <string>
#include <vector>
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/Vector4.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Math/IntVector2.hpp"

class Image;
class Vector2;
class Vector3;
class Material;
class MapChunk;
class Renderable;
class GameEntity;
class NPCTank;
class Bullet;
class Spawner;

struct MapVertex
{
	Vector3 position;
	Vector3 normal;
	Vector4 tangent;
	Vector2 uv;
};

struct RaycastHit_t
{
	RaycastHit_t(bool hitFlag, const Vector3& endPosition = Vector3::ZERO, bool wentFull = false)
	: hit(hitFlag), position(endPosition), wentFullDistance(wentFull) {}

	bool hit;
	Vector3 position;
	bool wentFullDistance;
};

class Map
{
public:
	//-----Public Methods-----

	Map() {}
	~Map();

	void Intialize(const AABB2& worldBounds, float minHeight, float maxHeight, const IntVector2& chunkLayout, const std::string& fileName);
	void Update();

	// Accessors


	// Producers
	Vector3 GetPositionAtVertexCoord(const IntVector2& vertexCoord) const;
	float	GetHeightAtVertexCoord(const IntVector2& vertexCoord) const;
	float	GetHeightAtPosition(const Vector3& position) const;
	Vector3 GetNormalAtVertexCoord(const IntVector2& vertexCoord) const;
	Vector3 GetNormalAtPosition(const Vector3& position) const;
	AABB2	GetWorldBounds() const;
	bool	IsPositionInCellBounds(const Vector3& position) const;

	std::vector<GameEntity*>& GetEntitiesOnMap();

	void AddGameEntity(GameEntity* entity);
	void KillAllEnemies();
	void DeleteObjectsMarkedForDelete();
	int		GetEnemyCount() const;
	std::vector<GameEntity*> GetEntitiesInArea(const Vector3& position, float radius);
	Vector3 GetPositionAwayFromEnemies() const;

	// Raycasts
	RaycastHit_t Raycast(const Vector3& startPosition, const Vector3& direction, float distance);

	// For swarming
	std::vector<GameEntity*> GetLocalSwarmers(const Vector3& relativePosition, float localDistance);


public:
	//-----Public Data-----

	static constexpr float MAX_RAYCAST_DISTANCE = 2000.f;


private:
	//-----Private Methods-----

	void BuildTerrain(Image* heightMap);
		void ConstructMapVertexList(Image* heightMap);
			void CalculateInitialPositionsAndUVs(Image* image);
		void BuildSingleChunk(int chunkXIndex, int chunkYIndex, Material* material);


	// Update
	void UpdateEntities();
	void CheckProjectilesAgainstActors();
	void CheckActorActorCollisions();
	void UpdateHeightAndOrientationOnMap();

	RaycastHit_t ConvergeRaycastOnTerrain(Vector3& positionBeforeHit, Vector3& positionAfterhit);
	RaycastHit_t ConvergeRaycastOnEntity(Vector3& positionBeforeHit, Vector3& positionAfterHit, const GameEntity* entity);


private:
	//-----Private Data-----

	AABB2					m_worldBounds;		// World-unit boundary
	IntVector2				m_mapCellLayout;	// Texel/Cell dimensions
	IntVector2				m_mapVertexLayout;	// +1 on X and Y off cell dimensions
	IntVector2				m_chunkLayout;		// Number of chunks wide/long

	FloatRange				m_heightRange;		// Max/Min height for the map
	std::vector<MapChunk*>	m_mapChunks;		// List of chunks
	Renderable*				m_waterRenderable;			// The plane for the water in the environment
	std::vector<MapVertex>	m_mapVertices;

	std::vector<GameEntity*>	m_gameEntities;

	static const int RAYCAST_CONVERGE_ITERATION_COUNT = 20;
	static constexpr float RAYCAST_CONVERGE_EARLYOUT_DISTANCE = 0.01f;

};
