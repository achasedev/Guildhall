#include "Engine/Assets/AssetDB.hpp"
#include "Game/Framework/VoxelGrid.hpp"
#include "Engine/Rendering/Core/Renderer.hpp"
#include "Engine/Math/IntVector3.hpp"
#include "Game/Framework/Game.hpp"
#include "Engine/Core/Time/ProfileLogScoped.hpp"

void VoxelGrid::Initialize(int width, int height, int length)
{
	m_dimensions = IntVector3(width, height, length);

	int numVoxels = width * height * length;
	m_voxels = (Rgba*) malloc(numVoxels * sizeof(Rgba));

}
#include "Engine/Core/EngineCommon.hpp"

void VoxelGrid::BuildMesh()
{
	ProfileLogScoped log("BuildMesh");
	UNUSED(log);

	static bool test = true;

	if (test)
	{
		MeshBuilder mb;
		mb.BeginBuilding(PRIMITIVE_TRIANGLES, true);

		// Iterate across all voxels, build a cube for each
		int numVoxels = GetVoxelCount();
		for (int i = 0; i < numVoxels; ++i)
		{
			//if (m_voxels[i].a != 0.f)
			//{
			// Get the position
			Vector3 position = GetPositionForIndex(i);

			mb.PushCube(position, Vector3::ONES, Rgba::GetRandomColor());
			//}
		}

		mb.FinishBuilding();
		mb.UpdateMesh<Vertex3D_PCU>(m_mesh);
	}

	//test = false;	
}

void VoxelGrid::Render()
{
	ProfileLogScoped log("VoxelGrid::Render");
	UNUSED(log);
	Renderer* renderer = Renderer::GetInstance();
	renderer->SetCurrentCamera(Game::GetGameCamera());
	//renderer->DrawVoxelGrid(IntVector3(GRID_SIZE, GRID_SIZE, GRID_SIZE), m_voxels, m_voxel);

	renderer->DrawMesh(&m_mesh);
}

int VoxelGrid::GetVoxelCount() const
{
	return m_dimensions.x * m_dimensions.y * m_dimensions.z;
}

Vector3 VoxelGrid::GetPositionForIndex(int index) const
{
	int numVoxelsPerLayer = m_dimensions.x * m_dimensions.z;
	int y = index / numVoxelsPerLayer;

	int leftoverInLayer = index % numVoxelsPerLayer;
	int z = leftoverInLayer / m_dimensions.x;
	int x = leftoverInLayer % m_dimensions.x;

	return Vector3(x, y, z);
}
