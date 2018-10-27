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

	void Inititalize();
	void CleanUp();

	void Update();
	void Render();

	// Mutators
	void AddEntity(Entity* entity);


	// Accessors
	IntVector3		GetDimensions() const;
	HeatMap*		GetNavMap() const;
	unsigned int	GetGroundElevation() const;

	// Producers
	Vector3		GetNextPositionTowardsPlayer(const Vector3& currPosition) const;
	bool		IsPositionInStatic(const Vector3& position) const;
	bool		HasLineOfSight(const Vector3& startPosition, const Vector3& endPosition) const;
	bool		IsEntityOnGround(const Entity* entity) const;

	// Utility
	void		ParticalizeEntity(Entity* entity);
	void		ParticalizeAllEntities();
	

private:
	//-----Private Methods-----

	// -- Update Loop -- 
	void UpdateCostMap();
	void UpdatePlayerHeatmap();

	void UpdateEntities();
	void UpdateParticles();

	void ApplyPhysicsStep();

	void CheckForGroundCollisions();
	void CheckStaticEntityCollisions();
	void CheckDynamicEntityCollisions();

	void DeleteMarkedEntities();

	// Render
	void DrawStaticEntitiesToGrid();
	void DrawDynamicEntitiesToGrid();
	void DrawParticlesToGrid();

	// Collision
	bool CheckAndCorrectEntityCollision(Entity* first, Entity* second);
	bool PerformBroadphaseCheck(Entity* first, Entity* second);
	bool PerformNarrowphaseCheck();


private:
	//-----Private Data-----

	IntVector3 m_dimensions;
	VoxelGrid*	m_voxelGrid;		
	unsigned int m_groundElevation = 0;

	std::vector<Entity*> m_entities;
	std::vector<Particle*> m_particles;

	HeatMap*		m_playerHeatmap = nullptr;
	HeatMap*		m_costsMap = nullptr;

	bool			m_drawCollision = false;

};
