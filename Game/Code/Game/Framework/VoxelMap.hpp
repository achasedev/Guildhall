/************************************************************************/
/* File: VoxelMap.hpp
/* Author: Andrew Chase
/* Date: November 25th 2018
/* Description: Class to represent the map of a world
/************************************************************************/
#pragma once
#include <map>
#include "Game/Entity/EntitySpawn.hpp"
#include "Engine/Math/IntVector3.hpp"
#include "Engine/Core/Utility/XmlUtilities.hpp"

class HeatMap;
class VoxelSprite;
class VoxelTerrain;
class MapDefinition;

class VoxelMap
{
public:
	//-----Public Methods-----
	
	static VoxelMap* CreateMapFromDefinition(const MapDefinition* mapDefinition);

	// Mutators
	void	AddVoxel(const IntVector3& coords, const Rgba& color);
	Rgba	RemoveVoxel(const IntVector3& coords);

	// Accessors
	int		GetHeightAtCoords(const IntVector2& coords);
	Rgba	GetColorAtCoords(const IntVector3& coords);
	
	
private:
	//-----Private Data-----
	
	// From Data
	const MapDefinition*	m_mapDefinition = nullptr;

	// State
	VoxelTerrain*			m_terrain = nullptr;
	HeatMap*				m_heightmap = nullptr;

	// Statics
	static const IntVector3 MAP_DIMENSIONS;	// All maps need to be this size

};
