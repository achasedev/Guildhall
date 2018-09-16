/************************************************************************/
/* File: World.hpp
/* Author: Andrew Chase
/* Date: September 15th, 2018
/* Description: Class to represent a game scene
/************************************************************************/
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

	VoxelGrid* m_voxelGrid;		// Grid used for all coloring

};
