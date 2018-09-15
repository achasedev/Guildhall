#include "Engine/Assets/AssetDB.hpp"
#include "Game/Framework/VoxelGrid.hpp"
#include "Engine/Rendering/Core/Renderer.hpp"
#include "Engine/Math/IntVector3.hpp"
#include "Game/Framework/Game.hpp"
#include "Engine/Core/Time/ProfileLogScoped.hpp"
#include "Engine/Rendering/Shaders/ComputeShader.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/MathUtils.hpp"

#define VERTICES_PER_VOXEL 24
#define INDICES_PER_VOXEL 36

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

	m_currentFrame = (Rgba*)malloc(numVoxels * sizeof(Rgba));
	m_previousFrame = (Rgba*)malloc(numVoxels * sizeof(Rgba));

	for (int i = 0; i < numVoxels; ++i)
	{
		m_currentFrame[i] = Rgba::GetRandomColor();
		IntVector3 coords = GetCoordsForIndex(i);

		if (CheckRandomChance(0.01f))
		{
			m_currentFrame[i].a = 0;
		}
	}

	m_chunks.resize(GetChunkCount());

	m_computeShader = new ComputeShader();
	m_computeShader->Initialize("Data/ComputeShaders/VoxelMeshRebuild.cs");

	m_buffers.Initialize(m_dimensions, chunkDimensions);
}

#include "Engine/Core/EngineCommon.hpp"

void VoxelGrid::Render()
{
	PROFILE_LOG_SCOPE_FUNCTION();

	// Set up our buffers
	UpdateBuffers();

	// Rebuild the meshes
	RebuildMeshes();

	// Draw the grid
	DrawGrid();
}

int VoxelGrid::GetVoxelCount() const
{
	return m_dimensions.x * m_dimensions.y * m_dimensions.z;
}

int VoxelGrid::GetChunkCount() const
{
	return m_chunkLayout.x * m_chunkLayout.y * m_chunkLayout.z;
}

unsigned int VoxelGrid::GetVoxelsPerChunk() const
{
	return m_chunkDimensions.x * m_chunkDimensions.y * m_chunkDimensions.z;
}

int VoxelGrid::GetIndexForCoords(const IntVector3& coords) const
{
	return coords.y * (m_dimensions.x * m_dimensions.z) + coords.z * m_dimensions.x + coords.x;
}

IntVector3 VoxelGrid::GetCoordsForIndex(unsigned int index) const
{
	int y = index / (m_dimensions.x * m_dimensions.z);
	int leftover = index % (m_dimensions.x * m_dimensions.z);

	int z = leftover / m_dimensions.x;
	int x = leftover % m_dimensions.x;

	return IntVector3(x, y, z);
}

#include "Engine/Input/InputSystem.hpp"

void VoxelGrid::UpdateBuffers()
{
	PROFILE_LOG_SCOPE_FUNCTION();

	Rgba temp = m_currentFrame[0];
	for (int i = 0; i < GetVoxelCount() - 1; ++i)
	{
		m_currentFrame[i] = m_currentFrame[i + 1];
	}

	m_currentFrame[GetVoxelCount() - 1] = temp;

	// Send down the color data
	m_buffers.m_colorBuffer.CopyToGPU(GetVoxelCount() * sizeof(Rgba), m_currentFrame);

	// Clear the offsets
	m_buffers.m_offsetBuffer.Clear(GetChunkCount() * sizeof(unsigned int));
}

void VoxelGrid::RebuildMeshes()
{
	PROFILE_LOG_SCOPE_FUNCTION();

	// Execute the build step
	m_computeShader->Execute(m_chunkLayout.x, m_chunkLayout.y, m_chunkLayout.z);

	// Get the data out and update the existing meshes
	unsigned int vertexHandle = m_buffers.m_vertexBuffer.GetHandle();
	unsigned int indexHandle = m_buffers.m_indexBuffer.GetHandle();
	
	unsigned int* offset = (unsigned int*)m_buffers.m_offsetBuffer.MapBufferData();
	//VertexVoxel* vertices = (VertexVoxel*)m_buffers.m_vertexBuffer.MapBufferData();

	{
		PROFILE_LOG_SCOPE("Sending Mesh Data");

		// Iterate across all meshes and update them
		for (int chunkIndex = 0; chunkIndex < 1; ++chunkIndex)
		{
			unsigned int faceOffset = offset[chunkIndex];

			unsigned int vertexCount = faceOffset * 4;
			unsigned int indexCount = faceOffset * 6;

			// Set the data
			//m_chunks[chunkIndex].SetVerticesFromGPUBuffer<VertexVoxel>(vertexCount, vertexHandle);
			//m_chunks[chunkIndex].SetIndicesFromGPUBuffer(indexCount, indexHandle);
			m_chunks[chunkIndex].m_vertexBuffer.m_handle = vertexHandle;
			m_chunks[chunkIndex].m_vertexBuffer.m_bufferSize = m_buffers.m_vertexBuffer.m_bufferSize;
			m_chunks[chunkIndex].m_vertexBuffer.m_vertexCount = vertexCount;
			m_chunks[chunkIndex].m_vertexBuffer.m_vertexLayout = &VertexVoxel::LAYOUT;
			m_chunks[chunkIndex].m_vertexLayout = &VertexVoxel::LAYOUT;

			m_chunks[chunkIndex].m_indexBuffer.m_indexCount = indexCount;
			m_chunks[chunkIndex].m_indexBuffer.m_indexStride = sizeof(VertexVoxel);
			m_chunks[chunkIndex].m_indexBuffer.m_handle = indexHandle;
			m_chunks[chunkIndex].m_indexBuffer.m_bufferSize = m_chunks[chunkIndex].m_indexBuffer.m_handle;

			//m_chunks[chunkIndex].SetVertices(vertexCount, vertices);
			//m_chunks[chunkIndex].SetIndices(indexCount, currIndices);
			m_chunks[chunkIndex].SetDrawInstruction(PRIMITIVE_TRIANGLES, true, 0, indexCount);
		}
	}

	// Unmap all the buffers
	m_buffers.m_offsetBuffer.UnmapBufferData();
	//m_buffers.m_vertexBuffer.UnmapBufferData();

	// Meshes are now updated
}
#include "Engine/Core/DeveloperConsole/DevConsole.hpp"

void VoxelGrid::DrawGrid()
{
	PROFILE_LOG_SCOPE_FUNCTION();

	// Draw
	Renderer* renderer = Renderer::GetInstance();
	renderer->SetCurrentCamera(Game::GetGameCamera());

	Renderable renderable;
	renderable.AddInstanceMatrix(Matrix44::IDENTITY);

	for (int i = 0; i < 1; ++i)
	{
		if (m_chunks[0].GetVertexBuffer()->GetVertexCount() > 0)
		{
			RenderableDraw_t draw;
			draw.sharedMaterial = AssetDB::GetSharedMaterial("Default_Opaque");
			draw.mesh = &m_chunks[0];

			renderable.AddDraw(draw);
		}
	}

	renderer->DrawRenderable(&renderable);
}
