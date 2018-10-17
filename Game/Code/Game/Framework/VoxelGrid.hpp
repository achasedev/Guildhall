/************************************************************************/
/* File: VoxelGrid.hpp
/* Author: Andrew Chase
/* Date: September 15th, 2018
/* Description: Class to represent a virtual 3D render display
/************************************************************************/
#pragma once
#include "Engine/Core/Rgba.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/IntVector3.hpp"
#include "Engine/Rendering/Meshes/Mesh.hpp"
#include "Engine/Rendering/Buffers/RenderBuffer.hpp"

class Renderable;
class ComputeShader;
class VoxelTexture;
class Entity;

class VoxelGrid
{
	friend class World;

public:
	//-----Public Methods-----

	// Initializers
	void				Initialize(const IntVector3& voxelDimensions);
	void				BuildMeshAndDraw();

	// Mutators
	void				Clear();
	void				DrawEntity(const Entity* entity);
	void				Draw3DTexture(const VoxelTexture* texture, const IntVector3& position);

	// Accessors
	int					GetVoxelCount() const;
	int					GetIndexForCoords(const IntVector3& coords) const;
	IntVector3			GetCoordsForIndex(unsigned int index) const;
	

private:
	//-----Private Methods-----

	// Initialization
	void				InitializeBuffers();

	// For the render step
	void				UpdateBuffers();
	void				RebuildMesh();
	void				DrawGrid();
	

private:
	//-----Private Data-----

	// Grid representation
	Rgba*					m_gridColors = nullptr;
	IntVector3				m_dimensions;

	// Meshbuilding
	RenderBuffer			m_colorBuffer;
	RenderBuffer			m_countBuffer;
	Mesh					m_mesh;
	ComputeShader*			m_computeShader = nullptr;

};
