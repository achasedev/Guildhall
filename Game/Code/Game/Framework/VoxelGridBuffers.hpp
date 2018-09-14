#pragma once
#include "Engine/Math/Vector3.hpp"
#include "Engine/Rendering/Buffers/ShaderStorageBuffer.hpp"
#include "Engine/Rendering/Core/Vertex.hpp"

class IntVector3;

class VoxelGridBuffers
{
public:
	//-----Public Methods-----

	void Initialize(const IntVector3& worldDimensions, const IntVector3& chunkDimensions);

	void SetupForDraw();
	void BindAll();

public:
	//-----Public Data-----

	ShaderStorageBuffer m_colorBuffer;
	ShaderStorageBuffer m_offsetBuffer;
	ShaderStorageBuffer m_vertexBuffer;
	ShaderStorageBuffer m_indexBuffer;

};