/************************************************************************/
/* File: World.hpp
/* Author: Andrew Chase
/* Date: September 15th, 2018
/* Description: Class to represent a game scene
/************************************************************************/
#pragma once
#include <vector>
#include <thread>
#include <shared_mutex>
#include "Engine/Core/Rgba.hpp"
#include "Engine/Math/IntRange.hpp"
#include "Engine/Math/IntVector3.hpp"
#include "Engine/Core/Utility/HeatMap.hpp"

#define MAX_PLAYERS (4)
#define NAV_STATIC_COST (9999.f)

class Player;
class VoxelGrid;
class Entity;
class VoxelTexture;
class Particle;
class HeatMap;
class VoxelFont;
class CampaignStage;

struct HeatMapSet_t
{
	HeatMap* m_navigationMap = nullptr;
	HeatMap* m_costMap = nullptr;
};

struct StaticSection_t
{
	StaticSection_t(const IntVector3& _coordPosition, const IntVector3& _dimensions)
		: coordPosition(_coordPosition), dimensions(_dimensions) {}

	IntVector3 coordPosition;
	IntVector3 dimensions;
};

class World
{
public:
	//-----Public Methods-----

	World();
	~World();

	void InitializeForMenu();
	void InititalizeForStage(CampaignStage* stage);
	void CleanUp();

	void Update();
	void Render();

	// Mutators
	void AddEntity(Entity* entity);
	void AddParticle(Particle* particle);
	void DestroyTerrain(const IntVector3& coord, const IntRange& radius = IntRange(0, 0));
	void SetTerrainHeightAtCoord(const IntVector3& coord, int height);

	// Accessors
	IntVector3			GetDimensions() const;
	HeatMap*			GetNavMap() const;
	unsigned int		GetGroundElevationAtCoord(const IntVector2& coord) const;
	const HeatMap*		GetHeightMap() const;


	// Producers
	IntVector3	GetCoordsForPosition(const Vector3& position) const;
	Vector3		GetNextPositionTowardsPlayer(const Vector3& currPosition) const;
	bool		IsPositionInStatic(const Vector3& position) const;
	bool		HasLineOfSight(const Vector3& startPosition, const Vector3& endPosition) const;
	bool		IsEntityOnGround(const Entity* entity) const;

	// Utility
	void						ParticalizeEntity(Entity* entity);
	void						ParticalizeAllEntities();
	bool						IsEntityOnMap(const Entity* entity) const;
	float						GetMapHeightForEntity(const Entity* entity) const;
	std::vector<const Entity*>	GetEnemiesWithinDistance(const Vector3& position, float radius) const;


private:
	//-----Private Methods-----

	// -- Update Loop -- 
	void UpdateCostMap();

	void UpdateEntities();
	void UpdateParticles();

	void ApplyPhysicsStep();

	void CheckStaticEntityCollisions();
	void CheckDynamicEntityCollisions();

	void ApplyCollisionCorrections();

	void DeleteMarkedEntities();

	// Render
	void DrawStaticEntitiesToGrid();
	void DrawDynamicEntitiesToGrid();
	void DrawParticlesToGrid();

	// Thread
	void InitializeHeatMaps();
	void HeatMapUpdate_Main();
	void HeatMapUpdate_Thread();
	void CleanUpHeatMaps();

	// Terrain
	void CheckEntityForGroundCollision(Entity* entity);


private:
	//-----Private Data-----

	const IntVector3 m_dimensions = IntVector3(256, 64, 256);

	// Terrain
	HeatMap m_heightMap;

	std::vector<Entity*> m_entities;
	std::vector<Particle*> m_particles;

	bool			m_drawCollision = false;
	bool			m_drawHeatmap = false;

	// For AI
	bool				m_isQuitting = false;
	HeatMapSet_t		m_navMapInUse;
	HeatMapSet_t		m_intermediateMap;
	HeatMapSet_t		m_backBufferMap;

	std::thread						m_heatMapThread;
	bool							m_swapReady = false;
	std::vector<StaticSection_t>	m_staticSectionsMain;
	std::vector<StaticSection_t>	m_staticSectionsThread;
	std::vector<IntVector3>			m_playerSeeds;

	std::shared_mutex				m_mapSwapLock;
};
