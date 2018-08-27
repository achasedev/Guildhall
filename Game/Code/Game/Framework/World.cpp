#include "Game/Framework/Game.hpp"
#include "Game/Framework/World.hpp"
#include "Game/Framework/VoxelGrid.hpp"
#include "Engine/Assets/AssetDB.hpp"
#include "Engine/Rendering/Core/Renderer.hpp"
#include "Engine/Core/Time/ProfileLogScoped.hpp"
#include "Engine/Core/EngineCommon.hpp"

World::World()
{
}

void World::Inititalize()
{
	m_voxelGrid = new VoxelGrid();
	m_voxelGrid->Initialize(16, 16, 16);
}

void World::Update()
{
	ProfileLogScoped log("World::Update");
	UNUSED(log);
	m_voxelGrid->BuildMesh();
}

void World::Render() const
{
	ProfileLogScoped log("World::Render");
	UNUSED(log);
	m_voxelGrid->Render();
}
