/************************************************************************/
/* File: Map.hpp
/* Author: Andrew Chase
/* Date: February 17th, 2017
/* Description: Class to represent a 3D block map for gameplay
/************************************************************************/
#pragma once
#include "Engine/Core/Image.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/IntVector3.hpp"

class Block;
class HeatMap;

class Map
{
public:
	//-----Public Methods-----

	Map();
	~Map();


	// Construction
	bool LoadFromHeightMap(const Image& heights, unsigned int maxHeight = 8);
	bool LoadFromHeightMap(const char* filename, unsigned int maxHeight = 8);


	// Core
	void Update();
	void Render();


	// Accessors
	int			GetNumBlocks() const { return m_dimensions.x * m_dimensions.y * m_dimensions.z; }
	int			GetMapWidth() const { return m_dimensions.x; }
	int			GetMapHeight() const { return m_dimensions.z; }
	int			GetMapDepth() const { return m_dimensions.y; }
	IntVector2	GetMapDimensions2D() const { return IntVector2(m_dimensions.x, m_dimensions.y); };
	Block*		GetBlock(const IntVector3& coords);
	HeatMap*	GetHeightMap() const;
	int			GetHeightAtMapCoords(const IntVector2& blockPosition) const;


	// Mutators
	void		SetBlockDefinitionByName(const IntVector3& coords, const char* definitionName);


	// Producers
	int			GetBlockIndex(const IntVector3& coords) const;
	Vector3		MapAngleToWorldDirection(float angle) const;
	bool		Are2DCoordsValid(const IntVector3& coords) const;
	bool		Are3DCoordsValid(const IntVector3& coords) const;
	bool		IsPositionInSolidBlock(const Vector3& position) const;
	IntVector3	WorldPositionToMapCoords(const Vector3& position) const;
	Vector3		MapCoordsToWorldPosition(const IntVector3& coords) const;
	Vector3		MapVectorToWorldVector(const IntVector3& mapVector) const;
	IntVector3	GetNeighborCoords(const IntVector3& startCoords, const IntVector2& stepDirection) const;

	// Utilities
	static bool AreSame2DCoordinate(const IntVector3& a, const IntVector3& b);
	

public:
	//-----Public Data-----

	const static IntVector3 MAP_STEP_NORTH;
	const static IntVector3 MAP_STEP_SOUTH;
	const static IntVector3 MAP_STEP_EAST;
	const static IntVector3 MAP_STEP_WEST;


private:
	//-----Private Methods-----

	std::vector<Block>	m_blocks;
	IntVector3			m_dimensions;
	HeatMap*			m_heightMap;

};
