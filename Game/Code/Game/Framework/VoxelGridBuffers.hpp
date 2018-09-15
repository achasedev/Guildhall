#pragma once
#include "Engine/Math/Vector3.hpp"
#include "Engine/Rendering/Buffers/RenderBuffer.hpp"
#include "Engine/Rendering/Core/Vertex.hpp"
#include "Engine/Rendering/Meshes/Mesh.hpp"

class IntVector3;

class VoxelGridBuffers
{
public:
	//-----Public Methods-----

	void Initialize(const IntVector3& worldDimensions, const IntVector3& chunkDimensions);

public:
	//-----Public Data-----

	RenderBuffer m_colorBuffer;
	RenderBuffer m_offsetBuffer;

	Mesh m_mesh;

	//VertexBuffer m_vertexBuffer;
	//IndexBuffer m_indexBuffer;

};
