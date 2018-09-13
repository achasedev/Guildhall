#pragma once

#include "Engine/Core/Rgba.hpp"
#include "Engine/Rendering/Buffers/RenderBuffer.hpp"
#include "Engine/Math/IntVector3.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Rendering/Meshes/Mesh.hpp"
#include "Engine/Rendering/Buffers/ShaderStorageBuffer.hpp"
#include "Game/Framework/VoxelMeshBuffer.hpp"

class Renderable;
class ComputeShader;

class VoxelGrid
{
public:

	void Initialize(const IntVector3& voxelDimensions, const IntVector3& chunkDimensions);
	void BuildMesh();
	void Render();

	int GetVoxelCount() const;
	int GetChunkCount() const;

	int GetIndexForCoords(const IntVector3& coords) const;
	Vector3 GetPositionForIndex(int index) const;

	void BuildChunk(int chunkIndex);

	bool IsVoxelEnclosed(const IntVector3& coords) const;

	
private:

	void RebuildMeshes();
	
private:

	Rgba*					m_currentFrame = nullptr;
	Rgba*					m_previousFrame = nullptr;

	IntVector3				m_dimensions;
	IntVector3				m_chunkDimensions;
	IntVector3				m_chunkLayout;
	std::vector<Mesh>		m_chunks;



	VoxelMeshBuffer			m_meshBuffer;
	ShaderStorageBuffer		m_colorBuffer;

	ComputeShader* m_computeShader = nullptr;
	//Mesh					m_mesh;
};
