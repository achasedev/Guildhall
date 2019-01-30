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
#include "Game/Entity/Entity.hpp"
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
class VoxelSprite;
class Particle;
class VoxelMap;
class VoxelFont;
class CampaignStage;
class MapDefinition;

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
	void ApplyExplosion(const IntVector3& coord, eEntityTeam team, int damage = 0, float radius = 0.f, float impulseMagnitude = 0.f, Entity* hitEntity = nullptr);
	void AddVoxelTomap(const IntVector3& coord, const Rgba& color);
	void SetBlockEdgeCollision(bool shouldBlock);

	// Accessors
	IntVector3			GetDimensions() const;
	int					GetGroundElevationAtCoord(const IntVector2& coord) const;
	int					GetCurrentMaxHeightOfmap() const;
	eTransitionEdge		GetDirectionToEnter() const;

	// Producers
	IntVector3	GetCoordsForPosition(const Vector3& position) const;
	bool		IsEntityOnGround(const Entity* entity) const;
	
	// Utility
	void					ParticalizeVoxelText(const std::string& text, const IntVector3& referenceStart, const VoxelFontDraw_t& options);
	void					ParticalizeEntity(Entity* entity);
	void					ParticalizeAllEntities();
	bool					IsEntityOnMap(const Entity* entity) const;
	bool					AreCoordsOnMap(const IntVector2& coords) const;
	bool					AreCoordsInWorld(const IntVector3& coords) const;
	int						GetMapHeightForEntity(const Entity* entity) const;
	int						GetMapHeightForBounds(const IntVector3& coordPosition, const IntVector2& dimensions) const;
	std::vector<Entity*>	GetEntitiesThatOverlapSphere(const Vector3& position, float radius) const;


private:
	//-----Private Methods-----

	void IntializeMap(const MapDefinition* mapDefinition);

	void UpdateEntities();
	void UpdateParticles();

	void ApplyPhysicsStep();

	void CheckDynamicEntityCollisions();
	void CheckStaticEntityCollisions();
	void CheckMapCollisions();
		void CheckGroundCollisions();
		void CheckEdgeCollisions();

	void ApplyCollisionCorrections();
	void DeleteMarkedEntities();
	void RespawnDeadPlayers();

	// Render
	void DrawStaticEntitiesToGrid(const IntVector3& offset);
	void DrawDynamicEntitiesToGrid(const IntVector3& offset);
	void DrawParticlesToGrid(const IntVector3& offset);

	// map
	void CheckEntityForGroundCollision(Entity* entity);

	// Explosions
	void DestroyPartOfMap(const IntVector3& coord, float radius = 0.f, float impulseMagnitude = 0.f);


private:
	//-----Private Data-----

	const IntVector3 m_dimensions = IntVector3(256, 64, 256);
	eTransitionEdge m_enterEdge = EDGE_WEST;

	// map
	VoxelMap* m_map = nullptr;

	std::vector<Entity*>	m_entities;
	std::vector<Particle*>	m_particles;

	bool					m_blockEdgeCollisions = true;
	bool					m_drawCollisions = false;

};
