#pragma once

#include "Engine/Core/Rgba.hpp"
#include "Engine/Rendering/Buffers/RenderBuffer.hpp"
#include "Engine/Math/IntVector3.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Rendering/Meshes/Mesh.hpp"

class Renderable;

class VoxelGrid
{
public:

	void Initialize(const IntVector3& voxelDimensions, const IntVector3& chunkLayout);
	void BuildMesh();
	void Render();

	int GetVoxelCount() const;
	int GetChunkCount() const;
	Vector3 GetPositionForIndex(int index) const;

	void BuildChunk(int chunkIndex);


private:

	Rgba*					m_currentFrame = nullptr;
	Rgba*					m_previousFrame = nullptr;

	IntVector3				m_dimensions;
	IntVector3				m_chunkDimensions;
	IntVector3				m_chunkLayout;
	std::vector<Mesh>		m_chunks;
	//Mesh					m_mesh;
};
