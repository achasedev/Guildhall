#pragma once
#include "Game/Framework/VoxelGridBuffers.hpp"
#include "Engine/Core/Rgba.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/IntVector3.hpp"
#include "Engine/Rendering/Meshes/Mesh.hpp"
#include "Engine/Rendering/Buffers/RenderBuffer.hpp"
#include "Engine/Rendering/Buffers/ShaderStorageBuffer.hpp"

class Renderable;
class ComputeShader;

class VoxelGrid
{
public:

	void Initialize(const IntVector3& voxelDimensions, const IntVector3& chunkDimensions);
	void Render();

	int GetVoxelCount() const;
	int GetChunkCount() const;

	unsigned int GetVoxelsPerChunk() const;

	int GetIndexForCoords(const IntVector3& coords) const;
	IntVector3 GetCoordsForIndex(unsigned int index) const;
	
private:

	void UpdateBuffers();
	void RebuildMeshes();
	void DrawGrid();
	
private:

	Rgba*					m_currentFrame = nullptr;
	Rgba*					m_previousFrame = nullptr;

	IntVector3				m_dimensions;
	IntVector3				m_chunkDimensions;
	IntVector3				m_chunkLayout;
	std::vector<Mesh>		m_chunks;

	VoxelGridBuffers		m_buffers;

	ComputeShader* m_computeShader = nullptr;
	//Mesh					m_mesh;
};
