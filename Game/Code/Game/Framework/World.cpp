/************************************************************************/
/* File: VoxelGrid.cpp
/* Author: Andrew Chase
/* Date: September 15th, 2018
/* Description: Implementation of the world class
/************************************************************************/
#include "Game/Entity/Player.hpp"
#include "Game/Framework/World.hpp"
#include "Game/Framework/VoxelGrid.hpp"
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
	m_testPlayer = new Player();
	m_voxelGrid = new VoxelGrid();
	m_voxelGrid->Initialize(IntVector3(256, 64, 256));
}


//-----------------------------------------------------------------------------------------------
// Update
//
void World::Update()
{
	PROFILE_LOG_SCOPE_FUNCTION();

	m_testPlayer->ProcessInput();
	m_testPlayer->Update();
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
	m_voxelGrid->Write3DTexture(m_testPlayer->GetPosition(), 0.f, m_testPlayer->GetTextureForOrientation());

	// Rebuild the mesh and draw it to screen
	m_voxelGrid->BuildMeshAndDraw();
}
