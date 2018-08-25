#pragma once

#include <vector>
#include "Engine/Core/Rgba.hpp"

class Entity;
class VoxelGrid;

class World
{
public:
	//-----Public Methods-----

	World();
	~World();

	void Inititalize();

	void Update();
	void Render() const;


private:
	//-----Private Data-----

	VoxelGrid* m_voxelGrid;
	std::vector<Entity*> m_entities;		// Entities in the scene outside of the environment blocks

};
