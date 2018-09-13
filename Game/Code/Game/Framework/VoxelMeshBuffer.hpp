#pragma once
#include "Engine/Math/Vector3.hpp"
#include "Engine/Rendering/Buffers/ShaderStorageBuffer.hpp"
#include "Engine/Rendering/Core/Vertex.hpp"

#define MAX_CHUNKS 8
#define MAX_VERTICES 4*4*4*6*4
#define MAX_INDICES 4*4*4*6*6

struct ChunkData_t
{
	VertexVoxel		vertexBuffer[MAX_VERTICES];
	unsigned int	indexBuffer[MAX_INDICES];

	size_t vertexCount;
	size_t indexCount;
	size_t dualHead;
};

struct VoxelWorldData_t
{
	ChunkData_t chunkData[MAX_CHUNKS];
};

class VoxelMeshBuffer
{
public:
	void Initialize();

	VoxelWorldData_t* MapMeshBufferData();
	void UnmapMeshBufferData();

private:
	

	ShaderStorageBuffer m_gpuBuffer;

};