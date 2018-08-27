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

	void Initialize(int width, int height, int length);
	void BuildMesh();
	void Render();

	int GetVoxelCount() const;
	Vector3 GetPositionForIndex(int index) const;

private:

	Rgba*					m_voxels = nullptr;

	IntVector3				m_dimensions;
	Mesh					m_mesh;
};
