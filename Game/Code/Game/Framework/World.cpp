/************************************************************************/
/* File: VoxelGrid.cpp
/* Author: Andrew Chase
/* Date: September 15th, 2018
/* Description: Implementation of the world class
/************************************************************************/
#include "Game/Framework/World.hpp"
#include "Game/Framework/VoxelGrid.hpp"


//-----------------------------------------------------------------------------------------------
// Constructor
//
World::World()
{
}


//-----------------------------------------------------------------------------------------------
// Destructor
//
World::~World()
{

}


//-----------------------------------------------------------------------------------------------
// Initializes the grid and any other setup
//
void World::Inititalize()
{
	m_voxelGrid = new VoxelGrid();
	m_voxelGrid->Initialize(IntVector3(128, 32, 128));
}


//-----------------------------------------------------------------------------------------------
// Update
//
void World::Update()
{
}


//-----------------------------------------------------------------------------------------------
// Render
//
void World::Render() const
{
	// Clear grid

	// Color in static geometry

	// Color in each entity (shouldn't overlap, this is after physics step)

	// Rebuild the mesh and draw it to screen
	m_voxelGrid->BuildMeshAndDraw();
}
