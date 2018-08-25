#include "Engine/Assets/AssetDB.hpp"
#include "Game/Framework/VoxelGrid.hpp"
#include "Engine/Rendering/Core/Renderer.hpp"
#include "Engine/Math/IntVector3.hpp"
#include "Game/Framework/Game.hpp"

void VoxelGrid::Initialize()
{
	m_voxel = new Renderable();
	m_voxel->AddInstanceMatrix(Matrix44::IDENTITY);

	RenderableDraw_t draw;
	draw.mesh = AssetDB::GetMesh("Cube");
	draw.sharedMaterial = AssetDB::CreateOrGetSharedMaterial("Data/Materials/Voxel_Instanced.material");
	m_voxel->AddDraw(draw);
	
	for (int i = 0; i < GRID_SIZE * GRID_SIZE * GRID_SIZE; ++i)
	{
		m_voxels[i] = Rgba::GetRandomColor();
	}
}

void VoxelGrid::Render() const
{
	Renderer* renderer = Renderer::GetInstance();
	renderer->SetCurrentCamera(Game::GetGameCamera());
	renderer->DrawVoxelGrid(IntVector3(GRID_SIZE, GRID_SIZE, GRID_SIZE), m_voxels, m_voxel);
}
