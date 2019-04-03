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
#include "Game/Entity/EntitySpawn.hpp"
#include "Game/Framework/VoxelGrid.hpp"
#include "Game/Framework/CampaignStage.hpp"

#include "Engine/Core/Rgba.hpp"
#include "Engine/Math/IntRange.hpp"
#include "Engine/Math/IntAABB2.hpp"
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

	void InititalizeForStage(const CampaignStage* stage);
	void IntializeMap(const MapDefinition* mapDefinition);
	void CleanUp();

	void Update();
	void DrawToGrid();
	void DrawToGridWithOffset(const IntVector3& offset);

	// Mutators
	void	AddEntity(Entity* entity);
	Entity* SpawnEntity(const EntityDefinition* definition, const Vector2& mapPosition, float orientation);
	void	RemoveEntity(Entity* entity);
	void	AddParticle(Particle* particle);
	void	ApplyExplosion(const IntVector3& coord, eEntityTeam team, int damage = 0, float radius = 0.f, float impulseMagnitude = 0.f, Entity* hitEntity = nullptr);
	void	AddVoxelToMap(const IntVector3& coord, const Rgba& color);
	void	DestroyPartOfMap(const IntVector3& coord, float radius = 0.f, float particleFlyAwaySpeed = 0.f, int maxYRadius = DEFAULT_MAX_Y_EXPLOSION_RADIUS);

	// Accessors
	IntVector3			GetDimensions() const;
	int					GetGroundElevationAtCoord(const IntVector2& coord) const;
	int					GetCurrentMaxHeightOfmap() const;
	eTransitionEdge		GetDirectionToEnter() const;
	float				GetCurrentGravityScale() const;

	// Producers
	IntVector3	GetCoordsForPosition(const Vector3& position) const;
	bool		IsEntityOnGround(const Entity* entity) const;
	
	// Utility
	void						ParticalizeVoxelText(const std::string& text, const IntVector3& referenceStart, const VoxelFontDraw_t& options);
	void						ParticalizeEntity(Entity* entity);
	void						ParticalizeAllEntities();
	bool						IsEntityOnMap(const Entity* entity) const;
	bool						AreCoordsOnMap(const IntVector2& coords) const;
	bool						AreCoordsInWorld(const IntVector3& coords) const;
	int							GetMapHeightForEntity(const Entity* entity) const;
	int							GetMapHeightForBounds(const IntVector3& coordPosition, const IntVector2& dimensions) const;
	int							GetMapHeightForPosition(const Vector2& position) const;
	std::vector<Entity*>		GetEntitiesThatOverlapSphere(const Vector3& position, float radius) const;
	const std::vector<Entity*>&	GetAllEntities();


private:
	//-----Private Methods-----

	void SpawnMapEntities(const MapDefinition* mapDefinition);
		bool FindSpawnLocation(const MapAreaSpawn_t& spawnArea, IntAABB2& out_spawnedArea, 
			const std::vector<IntAABB2>& areaOccupiedAreas, std::vector<IntAABB2>& globalOccupiedAreas);

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
	void CheckParticleForGroundCollision(Particle* particle);


private:
	//-----Private Data-----

	const IntVector3 m_dimensions = IntVector3(256, 64, 256);
	eTransitionEdge m_enterEdge = EDGE_WEST;

	// map
	VoxelMap*	m_map = nullptr;

	std::vector<Entity*>	m_entities;
	std::vector<Particle*>	m_particles;

	bool					m_drawCollisions = false;

	static constexpr int DEFAULT_MAX_Y_EXPLOSION_RADIUS = 4;

};
