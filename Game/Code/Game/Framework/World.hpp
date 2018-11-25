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
#include "Game/Framework/VoxelGrid.hpp"
#include "Game/Framework/CampaignStage.hpp"

#include "Engine/Core/Rgba.hpp"
#include "Engine/Math/IntRange.hpp"
#include "Engine/Math/IntVector3.hpp"
#include "Engine/Core/Utility/HeatMap.hpp"

#define MAX_PLAYERS (4)
#define NAV_STATIC_COST (9999.f)

class Player;
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
	void DrawToGrid();
	void DrawToGridWithOffset(const IntVector3& offset);

	// Mutators
	void AddEntity(Entity* entity);
	void RemoveEntity(Entity* entity);
	void AddParticle(Particle* particle);
	void DestroyTerrain(const IntVector3& coord, const IntRange& radius = IntRange(0, 0));
	void SetTerrainHeightAtCoord(const IntVector3& coord, int height);
	bool DecrementTerrainHeight(int decrementAmount);

	// Accessors
	IntVector3			GetDimensions() const;
	unsigned int		GetGroundElevationAtCoord(const IntVector2& coord) const;
	const HeatMap*		GetHeightMap() const;
	int					GetCurrentMaxHeightOfTerrain() const;
	eTransitionEdge		GetDirectionToEnter() const;

	// Producers
	IntVector3	GetCoordsForPosition(const Vector3& position) const;
	bool		IsEntityOnGround(const Entity* entity) const;
	Rgba		GetTerrainColorAtElevation(int elevation) const;

	// Utility
	void						ParticalizeVoxelText(const std::string& text, const IntVector3& referenceStart, const VoxelFontDraw_t& options);
	void						ParticalizeEntity(Entity* entity);
	void						ParticalizeAllEntities();
	bool						IsEntityOnMap(const Entity* entity) const;
	float						GetMapHeightForEntity(const Entity* entity) const;
	float						GetMapHeightForBounds(const IntVector3& coordPosition, const IntVector2& dimensions) const;
	std::vector<const Entity*>	GetEnemiesWithinDistance(const Vector3& position, float radius) const;


private:
	//-----Private Methods-----

	void UpdateEntities();
	void UpdateParticles();

	void ApplyPhysicsStep();

	void CheckStaticEntityCollisions();
	void CheckDynamicEntityCollisions();

	void ApplyCollisionCorrections();

	void DeleteMarkedEntities();

	// Render
	void DrawStaticEntitiesToGrid(const IntVector3& offset);
	void DrawDynamicEntitiesToGrid(const IntVector3& offset);
	void DrawParticlesToGrid(const IntVector3& offset);

	// Terrain
	void CheckEntityForGroundCollision(Entity* entity);


private:
	//-----Private Data-----

	const IntVector3 m_dimensions = IntVector3(256, 64, 256);
	eTransitionEdge m_enterEdge = EDGE_WEST;

	// Terrain
	HeatMap m_heightMap;

	std::vector<Entity*> m_entities;
	std::vector<Particle*> m_particles;

	bool			m_drawCollision = false;
	bool			m_drawHeatmap = false;

};
