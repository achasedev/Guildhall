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
#include <string>


class Renderable;
class ComputeShader;
class VoxelTexture;
class Entity;
class HeatMap;
class VoxelFont;

enum eFillMode
{
	FILL_MODE_NONE,
	FILL_MODE_TOTAL,
	FILL_MODE_EDGE
};

struct VoxelFontDraw_t
{
	const VoxelFont* font;

	Rgba color;
	eFillMode mode = FILL_MODE_NONE;
	Rgba optionColor;

	int borderThickness = 0;

	IntVector3 right = IntVector3(1, 0, 0);
	IntVector3 up = IntVector3(0, 1, 0);

	Vector3 alignment = Vector3::ZERO;

	IntVector3 scale = IntVector3::ONES;
};

class VoxelGrid
{
	friend class World;

public:
	//-----Public Methods-----

	~VoxelGrid();

	// Initializers
	void				Initialize(const IntVector3& voxelDimensions);
	void				BuildMeshAndDraw();

	// Mutators
	void				Clear();
	void				DrawEntity(const Entity* entity);
	void				DrawTerrain(HeatMap* heightMap);
	void				Draw3DTexture(const VoxelTexture* texture, const IntVector3& startCoord);

	void				DebugDrawEntityCollision(const Entity* entity);

	void				DrawVoxelText(const std::string& text, const IntVector3& startCoord, const VoxelFontDraw_t& options);

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
