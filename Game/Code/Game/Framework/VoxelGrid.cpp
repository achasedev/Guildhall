#include "Engine/Assets/AssetDB.hpp"
#include "Game/Framework/VoxelGrid.hpp"
#include "Engine/Rendering/Core/Renderer.hpp"
#include "Engine/Math/IntVector3.hpp"
#include "Game/Framework/Game.hpp"
#include "Engine/Core/Time/ProfileLogScoped.hpp"

void VoxelGrid::Initialize(const IntVector3& voxelDimensions, const IntVector3& chunkDimensions)
{
	m_dimensions = voxelDimensions;
	m_chunkDimensions = chunkDimensions;

	m_chunkLayout = IntVector3(
		m_dimensions.x / m_chunkDimensions.x,
		m_dimensions.y / m_chunkDimensions.y, 
		m_dimensions.z / m_chunkDimensions.z
	);


	int numVoxels = m_dimensions.x * m_dimensions.y * m_dimensions.z;

	m_currentFrame = (Rgba*) malloc(numVoxels * sizeof(Rgba));

	for (int i = 0; i < numVoxels; ++i)
	{
		m_currentFrame[i] = Rgba::GetRandomColor();
	}

	m_chunks.resize(GetChunkCount());
}
#include "Engine/Core/EngineCommon.hpp"

void VoxelGrid::BuildMesh()
{
	ProfileLogScoped log("BuildMesh");
	UNUSED(log);

	static bool test = true;

	if (test)
	{
		int chunkCount = GetChunkCount();
		for (int chunkIndex = 0; chunkIndex < chunkCount; ++chunkIndex)
		{
			BuildChunk(chunkIndex);
		}
	}

	test = false;	
}

void VoxelGrid::Render()
{
	ProfileLogScoped log("VoxelGrid::Render");
	UNUSED(log);
	Renderer* renderer = Renderer::GetInstance();
	renderer->SetCurrentCamera(Game::GetGameCamera());
	//renderer->DrawVoxelGrid(IntVector3(GRID_SIZE, GRID_SIZE, GRID_SIZE), m_voxels, m_voxel);

	Renderable renderable;
	renderable.AddInstanceMatrix(Matrix44::IDENTITY);

	for (int i = 0; i < GetChunkCount(); ++i)
	{
		if (m_chunks[i].GetVertexBuffer()->GetVertexCount() > 0)
		{
			RenderableDraw_t draw;
			draw.sharedMaterial = AssetDB::GetSharedMaterial("Default_Opaque");
			draw.mesh = &m_chunks[i];

			renderable.AddDraw(draw);
		}
	}
	
	renderer->DrawRenderable(&renderable);
}

int VoxelGrid::GetVoxelCount() const
{
	return m_dimensions.x * m_dimensions.y * m_dimensions.z;
}

int VoxelGrid::GetChunkCount() const
{
	return m_chunkLayout.x * m_chunkLayout.y * m_chunkLayout.z;
}

int VoxelGrid::GetIndexForCoords(const IntVector3& coords) const
{
	return coords.y * (m_dimensions.x * m_dimensions.z) + coords.z * m_dimensions.x + coords.x;
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

void VoxelGrid::BuildChunk(int chunkIndex)
{
	int yOffset = (chunkIndex / (m_chunkLayout.x * m_chunkLayout.z)) * m_chunkDimensions.y;

	int leftOver = chunkIndex % (m_chunkLayout.x * m_chunkLayout.z);
	int zOffset = (leftOver / m_chunkLayout.x) * m_chunkDimensions.z;

	int xOffset = (leftOver % m_chunkLayout.x) * m_chunkDimensions.x;

	MeshBuilder mb;
	mb.BeginBuilding(PRIMITIVE_TRIANGLES, true);

	for (int i = 0; i < m_chunkDimensions.x; ++i)
	{
		for (int j = 0; j < m_chunkDimensions.y; ++j)
		{
			for (int k = 0; k < m_chunkDimensions.z; ++k)
			{
				IntVector3 coords = IntVector3(i + xOffset, j + yOffset, k + zOffset);

				int index = coords.y * (m_dimensions.x * m_dimensions.z) + coords.z * m_dimensions.x + coords.x;

				if (m_currentFrame[index].a != 0.f)
				{
					// Check if we're enclosed
					if (!IsVoxelEnclosed(coords))
					{
						// Get the position
						Vector3 position = GetPositionForIndex(index);
						mb.PushCube(position, Vector3::ONES, m_currentFrame[index]);
					}
				}
			}
		}
	}

	mb.FinishBuilding();
	mb.UpdateMesh(m_chunks[chunkIndex]);
}

bool VoxelGrid::IsVoxelEnclosed(const IntVector3& coords) const
{
	if (coords.x == 0 || coords.y == 0 || coords.z == 0)
	{
		return false;
	}

	if (coords.x == m_dimensions.x - 1 || coords.y == m_dimensions.y - 1 || coords.z == m_dimensions.z - 1)
	{
		return false;
	}

	// Check neighbors
	IntVector3 north = coords + IntVector3(0, 0, 1);
	IntVector3 south = coords + IntVector3(0, 0, -1);
	IntVector3 east = coords + IntVector3(0, 0, 1);
	IntVector3 west = coords + IntVector3(0, 0, -1);
	IntVector3 up = coords + IntVector3(0, 1, 0);
	IntVector3 down = coords + IntVector3(0, -1, 0);

	if (m_currentFrame[GetIndexForCoords(north)].a == 0)	{ return false; }
	if (m_currentFrame[GetIndexForCoords(south)].a == 0)	{ return false; }
	if (m_currentFrame[GetIndexForCoords(east)].a == 0)		{ return false; }
	if (m_currentFrame[GetIndexForCoords(west)].a == 0)		{ return false; }
	if (m_currentFrame[GetIndexForCoords(up)].a == 0)		{ return false; }
	if (m_currentFrame[GetIndexForCoords(down)].a == 0)		{ return false; }

	return true;
}
