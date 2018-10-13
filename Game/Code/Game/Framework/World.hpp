/************************************************************************/
/* File: World.hpp
/* Author: Andrew Chase
/* Date: September 15th, 2018
/* Description: Class to represent a game scene
/************************************************************************/
#pragma once
#include <vector>
#include "Engine/Core/Rgba.hpp"
#include "Engine/Math/IntVector3.hpp"

#define MAX_PLAYERS (4)
#define NAV_DIMENSION_FACTOR (4)
#define NAV_STATIC_COST (9999.f)

class Player;
class VoxelGrid;
class Entity;
class VoxelTexture;
class Particle;
class HeatMap;

class World
{
public:
	//-----Public Methods-----

	World();
	~World();

	void Inititalize(const char* filename);

	void Update();
	void Render();

	// Mutators
	void AddEntity(Entity* entity);
	void ParticalizeAllEntities();

	// Accessors
	IntVector3	GetDimensions() const;
	HeatMap*	GetNavMap() const;

	// Producers
	Vector3		GetNextPositionTowardsPlayer(const Vector3& currPosition) const;


private:
	//-----Private Methods-----

	// -- Update Loop -- 
	void UpdateCostMap();
	void UpdatePlayerHeatmap();

	void UpdateEntities();
	void UpdateParticles();

	void ApplyPhysicsStep();

	void CheckStaticEntityCollisions();
	void CheckDynamicEntityCollisions();

	void DeleteMarkedEntities();

	// Render
	void DrawTerrainToGrid();
	void DrawStaticEntitiesToGrid();
	void DrawDynamicEntitiesToGrid();
	void DrawParticlesToGrid();

	// Collision
	bool CheckAndCorrectEntityCollision(Entity* first, Entity* second);
	bool CheckAndCorrect_DiscDisc(Entity* first, Entity* second);
	bool CheckAndCorrect_BoxDisc(Entity* first, Entity* second);
	bool CheckAndCorrect_BoxBox(Entity* first, Entity* second);


private:
	//-----Private Data-----

	IntVector3 m_dimensions;
	VoxelGrid*	m_voxelGrid;		
	unsigned int m_groundElevation = 0;

	VoxelTexture* m_terrain = nullptr;
	std::vector<Entity*> m_entities;
	std::vector<Particle*> m_particles;

	HeatMap*		m_playerHeatmap = nullptr;
	HeatMap*		m_costsMap = nullptr;

};
