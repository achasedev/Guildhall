#include "Game/Framework/Game.hpp"
#include "Game/Framework/World.hpp"
#include "Game/Framework/VoxelGrid.hpp"
#include "Engine/Assets/AssetDB.hpp"
#include "Engine/Rendering/Core/Renderer.hpp"
World::World()
{

}

void World::Inititalize()
{
	m_voxelGrid = new VoxelGrid();
	m_voxelGrid->Initialize();

}

void World::Render() const
{
	m_voxelGrid->Render();
}
