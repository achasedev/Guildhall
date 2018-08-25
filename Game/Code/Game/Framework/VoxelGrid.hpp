#pragma once

#include "Engine/Core/Rgba.hpp"
#include "Engine/Rendering/Buffers/RenderBuffer.hpp"

#define GRID_SIZE 128

class Renderable;

class VoxelGrid
{
public:

	void Initialize();
	void Render() const;

private:

	Renderable*				m_voxel;
	Rgba					m_voxels[GRID_SIZE * GRID_SIZE * GRID_SIZE];
	mutable RenderBuffer	m_colorBuffer;

};
