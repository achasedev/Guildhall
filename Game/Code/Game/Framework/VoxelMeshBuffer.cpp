#include "Game/Framework/VoxelMeshBuffer.hpp"

void VoxelMeshBuffer::Initialize()
{
	void* data = malloc(sizeof(VoxelWorldData_t));
	memset(data, 0, sizeof(VoxelWorldData_t));

	m_gpuBuffer.Bind(11);
	m_gpuBuffer.CopyToGPU(sizeof(VoxelWorldData_t), data);

	delete data;
}

VoxelWorldData_t* VoxelMeshBuffer::MapMeshBufferData()
{
	return (VoxelWorldData_t*)m_gpuBuffer.MapBufferData();
}

void VoxelMeshBuffer::UnmapMeshBufferData()
{
	m_gpuBuffer.UnmapBufferData();
}

