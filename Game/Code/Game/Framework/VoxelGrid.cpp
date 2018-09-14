#include "Engine/Assets/AssetDB.hpp"
#include "Game/Framework/VoxelGrid.hpp"
#include "Engine/Rendering/Core/Renderer.hpp"
#include "Engine/Math/IntVector3.hpp"
#include "Game/Framework/Game.hpp"
#include "Engine/Core/Time/ProfileLogScoped.hpp"
#include "Engine/Rendering/Shaders/ComputeShader.hpp"

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

	m_currentFrame = (Rgba*) malloc(numVoxels * sizeof(Rgba));
	m_previousFrame = (Rgba*)malloc(numVoxels * sizeof(Rgba));

	for (int i = 0; i < numVoxels; ++i)
	{
		m_currentFrame[i] = Rgba::GetRandomColor();
		m_previousFrame[i] = Rgba::BLUE;
	}

	m_chunks.resize(GetChunkCount());

	m_computeShader = new ComputeShader();
	m_computeShader->Initialize("Data/ComputeShaders/VoxelMeshRebuild.cs");

	m_buffers.Initialize(m_dimensions, chunkDimensions);
}

#include "Engine/Core/EngineCommon.hpp"

void VoxelGrid::Render()
{
	static bool test = true;

	if (true)
	{
		// Set up our buffers
		UpdateBuffers();

		// Rebuild the meshes
		RebuildMeshes();

		test = false;
	}


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

#include "Engine/Input/InputSystem.hpp"

void VoxelGrid::UpdateBuffers()
{
	static bool test = true;

	if (InputSystem::GetInstance()->IsKeyPressed('I'))
	{
		test = !test;
	}

	Rgba* colorsource = (test ? m_currentFrame : m_previousFrame);

	// Send down the color data
	m_buffers.m_colorBuffer.CopyToGPU(GetVoxelCount() * sizeof(Rgba), colorsource);

	// Clear the offsets
	m_buffers.m_offsetBuffer.Clear(GetChunkCount() * sizeof(unsigned int));
}

void VoxelGrid::RebuildMeshes()
{
	// Execute the build step
	m_computeShader->Execute(m_chunkLayout.x, m_chunkLayout.y, m_chunkLayout.z);

	// Get the data out and update the existing meshes
	VertexVoxel* vertices = (VertexVoxel*)m_buffers.m_vertexBuffer.MapBufferData();
	unsigned int* indices = (unsigned int*)m_buffers.m_indexBuffer.MapBufferData();
	unsigned int* offset = (unsigned int*)m_buffers.m_offsetBuffer.MapBufferData();

	// Iterate across all meshes and update them
	for (int chunkIndex = 0; chunkIndex < GetChunkCount(); ++chunkIndex)
	{
		VertexVoxel* currVertices = vertices + (chunkIndex * GetVoxelsPerChunk() * VERTICES_PER_VOXEL);
		unsigned int* currIndices = indices + (chunkIndex * GetVoxelsPerChunk() * INDICES_PER_VOXEL);

		unsigned int combinedOffset = offset[chunkIndex];

		unsigned int vertexCount = combinedOffset >> 16;
		unsigned int indexCount = combinedOffset & 0xFFFF;

		m_chunks[chunkIndex].SetVertices(vertexCount, currVertices);
		m_chunks[chunkIndex].SetIndices(indexCount, currIndices);
		m_chunks[chunkIndex].SetDrawInstruction(PRIMITIVE_TRIANGLES, true, 0, indexCount);
	}

	// Unmap all the buffers
	m_buffers.m_offsetBuffer.UnmapBufferData();
	m_buffers.m_indexBuffer.UnmapBufferData();
	m_buffers.m_vertexBuffer.UnmapBufferData();

	// Meshes are now updated
}

void VoxelGrid::DrawGrid()
{
	// Draw
	Renderer* renderer = Renderer::GetInstance();
	renderer->SetCurrentCamera(Game::GetGameCamera());

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
