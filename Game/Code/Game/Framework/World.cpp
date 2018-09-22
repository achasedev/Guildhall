/************************************************************************/
/* File: VoxelGrid.cpp
/* Author: Andrew Chase
/* Date: September 15th, 2018
/* Description: Implementation of the world class
/************************************************************************/
#include "Game/Framework/World.hpp"
#include "Game/Framework/VoxelGrid.hpp"
#include "Game/Entity/Entity.hpp"
#include "Engine/Core/Time/ProfileLogScoped.hpp"

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
	m_testEntity = new Entity();
	m_voxelGrid = new VoxelGrid();
	m_voxelGrid->Initialize(IntVector3(256, 64, 256));
}


//-----------------------------------------------------------------------------------------------
// Update
//
void World::Update()
{
	PROFILE_LOG_SCOPE_FUNCTION();

	m_testEntity->Update();
}


//-----------------------------------------------------------------------------------------------
// Render
//
void World::Render() const
{
	PROFILE_LOG_SCOPE_FUNCTION();

	// Clear grid
	m_voxelGrid->Clear();

	// Color in static geometry

	// Color in each entity (shouldn't overlap, this is after physics step)
	m_voxelGrid->Write3DTexture(Vector3(20.f, 0.f, 20.f), 0.f, m_testEntity->GetTextureForOrientation());

	// Rebuild the mesh and draw it to screen
	m_voxelGrid->BuildMeshAndDraw();
}
