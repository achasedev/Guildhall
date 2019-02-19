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

class ComputeShader;
class VoxelSprite;
class Entity;
class HeatMap;
class VoxelFont;
class VoxelMap;
class VoxelMetaData;

enum eVoxelFontFill
{
	VOXEL_FONT_FILL_NONE,
	VOXEL_FONT_FILL_FULL,
	VOXEL_FONT_FILL_EDGE
};

typedef IntVector3(*VoxelFontOffset_cb)(const IntVector3& localCoords, const IntVector3& worldCoords, void* args);
typedef Rgba(*VoxelFontColor_cb)(const IntVector3& localCoords, const IntVector3& worldCoords, const Rgba& baseColor, void* args);

struct VoxelFontDraw_t
{
	const VoxelFont* font;

	std::vector<Rgba> glyphColors;
	eVoxelFontFill mode = VOXEL_FONT_FILL_NONE;
	Rgba fillColor;

	int borderThickness = 0;

	IntVector3 right = IntVector3(1, 0, 0);
	IntVector3 up = IntVector3(0, 1, 0);
	IntVector3 scale = IntVector3::ONES;
	Vector3 alignment = Vector3::ZERO;

	VoxelFontOffset_cb offsetFunction = nullptr;
	void* offsetFunctionArgs = nullptr;

	VoxelFontColor_cb colorFunction = nullptr;
	void* colorFunctionArgs = nullptr;
};


struct VoxelDrawOptions_t
{
	bool receivesShadows = false;
	bool castsShadows = false;
	Rgba whiteReplacement = Rgba::WHITE;
	
	bool hasColorOverride = false;
	Rgba colorOverride = Rgba::WHITE;
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
	void				ColorVoxelAtCoords(const IntVector3& coords, const Rgba& color);
	void				DrawEntity(const Entity* entity, const IntVector3& offset, VoxelDrawOptions_t options = VoxelDrawOptions_t());
	void				DrawEntityCollision(const Entity* entity, const IntVector3& offset);
	void				DrawMap(VoxelMap* map, const IntVector3& offset);
	void				DrawVoxelSprite(const VoxelSprite* texture, const IntVector3& startCoord, float orientation, VoxelDrawOptions_t options = VoxelDrawOptions_t());

	void				DebugDrawEntityCollision(const Entity* entity, const IntVector3& offset);

	void				DrawVoxelText(const std::string& text, const IntVector3& startCoord, const VoxelFontDraw_t& options);

	void				DrawWireBox(const IntVector3& startCoords, const IntVector3& dimensions, const Rgba& color,
										bool shadeX = false, bool shadeY = false, bool shadeZ = false);
	void				DrawSolidBox(const IntVector3& startCoords, const IntVector3& dimensions, const Rgba& color, bool overwrite = true);

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
	VoxelMetaData*			m_metaData = nullptr;
	IntVector3				m_dimensions;

	// Meshbuilding
	RenderBuffer			m_colorBuffer;
	RenderBuffer			m_metaBuffer;
	RenderBuffer			m_countBuffer;
	Mesh					m_mesh;
	ComputeShader*			m_computeShader = nullptr;

};


//-----------------------------------------------------------------------------------------------
// Returns the linear index for the voxel given by coords
//
inline int VoxelGrid::GetIndexForCoords(const IntVector3& coords) const
{
	// Check if it's in bounds first
	if (coords.x >= m_dimensions.x || coords.x < 0 || coords.y >= m_dimensions.y || coords.y < 0 || coords.z >= m_dimensions.z || coords.z < 0)
	{
		return -1;
	}

	return (coords.y * m_dimensions.x * m_dimensions.z) + (coords.z * m_dimensions.x) + coords.x;
}