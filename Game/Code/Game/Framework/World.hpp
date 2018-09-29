/************************************************************************/
/* File: World.hpp
/* Author: Andrew Chase
/* Date: September 15th, 2018
/* Description: Class to represent a game scene
/************************************************************************/
#pragma once
#include <vector>
#include "Engine/Core/Rgba.hpp"

#define MAX_PLAYERS (4)

class Player;
class VoxelGrid;
class Entity;
class DynamicEntity;
class StaticEntity;

class World
{
public:
	//-----Public Methods-----

	World();
	~World();

	void Inititalize();

	void Update();
	void Render();

	void AddDynamicEntity(DynamicEntity* entity);
	void AddStaticEntity(StaticEntity* entity);


private:
	//-----Private Methods-----

	// -- Update Loop -- 
	void UpdateStaticEntities();
	void UpdateDynamicEntities();

	void ApplyPhysicsStep();

	void CheckStaticEntityCollisions();
	void CheckDynamicEntityCollisions();

	void DeleteMarkedEntities();

	// Render
	void DrawStaticEntitiesToGrid();
	void DrawDynamicEntitiesToGrid();

	// Collision
	bool CheckAndCorrectEntityCollision(Entity* first, Entity* second);
	bool CheckAndCorrect_DiscDisc(Entity* first, Entity* second);
	bool CheckAndCorrect_BoxDisc(Entity* first, Entity* second);
	bool CheckAndCorrect_BoxBox(Entity* first, Entity* second);


private:
	//-----Private Data-----

	VoxelGrid*	m_voxelGrid;		
	unsigned int m_groundElevation = 0;

	std::vector<DynamicEntity*> m_dynamicEntities;
	std::vector<StaticEntity*>	m_staticEntities;

};
