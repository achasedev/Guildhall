#include "Game/Framework/Game.hpp"
#include "Game/Framework/World.hpp"
#include "Game/Framework/VoxelGrid.hpp"
#include "Engine/Assets/AssetDB.hpp"
#include "Engine/Rendering/Core/Renderer.hpp"
#include "Engine/Core/Time/ProfileLogScoped.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/MathUtils.hpp"

World::World()
{
}

void World::Inititalize()
{
	m_voxelGrid = new VoxelGrid();
	m_voxelGrid->Initialize(IntVector3(256, 64, 256), IntVector3(8, 8, 8));
}

void World::Update()
{
	ProfileLogScoped log("World::Update");
	UNUSED(log);

	// Rebuild stuff
// 	for (int i = 0; i < m_voxelGrid->GetChunkCount() / 16; ++i)
// 	{
// 		int index = GetRandomIntLessThan(m_voxelGrid->GetChunkCount());
// 		m_voxelGrid->BuildChunk(index);
// 	}
}

void World::Render() const
{
	ProfileLogScoped log("World::Render");
	UNUSED(log);
	m_voxelGrid->Render();
}
