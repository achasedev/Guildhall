/************************************************************************/
/* File: VoxelMap.cpp
/* Author: Andrew Chase
/* Date: January 31st 2019
/* Description: IMplementation of the VoxelMap class
/************************************************************************/
#include "Game/Framework/VoxelMap.hpp"
#include "Game/Framework/GameCommon.hpp"
#include "Game/Animation/VoxelSprite.hpp"
#include "Game/Animation/VoxelSprite.hpp"
#include "Game/Framework/VoxelTerrain.hpp"
#include "Game/Entity/EntityDefinition.hpp"
#include "Game/Framework/MapDefinition.hpp"

#include "Engine/Core/Rgba.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Utility/HeatMap.hpp"
#include "Engine/Core/Utility/XmlUtilities.hpp"


const IntVector3 VoxelMap::MAP_DIMENSIONS = IntVector3(256, 64, 256);

//- C FUNCTION ----------------------------------------------------------------------------------
// Returns the height of the terrain at the given coords within the sprite (terrain)
// Height defined as the y index of the first non-solid voxel at a given xz-coords
//
int GetSpriteHeightAtCoords(VoxelSprite* sprite, const IntVector2& coords, const IntVector3& spriteDimensions)
{
	for (int y = 0; y < spriteDimensions.y; ++y)
	{
		Rgba color = sprite->GetColorAtRelativeCoords(IntVector3(coords.x, y, coords.y), 0.f);
		if (color.a == 0)
		{
			return y;
		}
	}

	return spriteDimensions.y;
}


//-----------------------------------------------------------------------------------------------
// Constructs and initializes a map with the given map definition
//
VoxelMap* VoxelMap::CreateMapFromDefinition(const MapDefinition* mapDefinition)
{
	ASSERT_OR_DIE(mapDefinition != nullptr, "VoxelMap::CreateMapFromDefinition() was given a nullptr definition");

	VoxelMap* map = new VoxelMap();
	map->m_mapDefinition = mapDefinition;

	// Initialize the terrain by using either the type or the name
	if (!IsStringNullOrEmpty(mapDefinition->m_terrainName))
	{
		map->m_terrain = VoxelTerrain::CreateVoxelTerrainCloneForName(mapDefinition->m_terrainName);
	}
	else
	{
		map->m_terrain = VoxelTerrain::CreateVoxelTerrainCloneForType(mapDefinition->m_terrainType);
	}

	// Create the heightmap
	map->m_heightmap = new HeatMap(MAP_DIMENSIONS.xz(), 0.f);

	for (int z = 0; z < MAP_DIMENSIONS.z; ++z)
	{
		for (int x = 0; x < MAP_DIMENSIONS.x; ++x)
		{
			IntVector2 coords = IntVector2(x, z);
			map->m_heightmap->SetHeat(coords, (float)GetSpriteHeightAtCoords(map->m_terrain, coords, MAP_DIMENSIONS));
		}
	}

	return map;
}


//-----------------------------------------------------------------------------------------------
// Colors in the voxel at the given coordinates with the given color, updating the height map
// if appropriate
//
void VoxelMap::AddVoxel(const IntVector3& coords, const Rgba& color)
{
	m_terrain->SetColorAtRelativeCoords(coords, 0.f, color);

	// Update the height map if applicable
	int oldHeight = (int)m_heightmap->GetHeat(coords.xz());
	if (coords.y == oldHeight)
	{
		m_heightmap->SetHeat(coords.xz(), (float)(oldHeight + 1));
	}
}

//-----------------------------------------------------------------------------------------------
// Clears the voxel at the given coords, updating the height map if appropriate
//
Rgba VoxelMap::RemoveVoxel(const IntVector3& coords)
{
	Rgba color = m_terrain->GetColorAtRelativeCoords(coords, 0.f);
	m_terrain->SetColorAtRelativeCoords(coords, 0.f, Rgba(0, 0, 0, 0));

	// Update the height map if applicable
	int oldHeight = (int)m_heightmap->GetHeat(coords.xz());
	if (coords.y == oldHeight - 1)
	{
		m_heightmap->SetHeat(coords.xz(), (float)(oldHeight - 1));
	}

	return color;
}


//-----------------------------------------------------------------------------------------------
// Returns the heighmap value (as int) at the given xz-coords
//
int VoxelMap::GetHeightAtCoords(const IntVector2& coords)
{
	return (int)m_heightmap->GetHeat(coords);
}


//-----------------------------------------------------------------------------------------------
// Returns the color of the voxel at the given map coordinates
//
Rgba VoxelMap::GetColorAtCoords(const IntVector3& coords)
{
	return m_terrain->GetColorAtRelativeCoords(coords, 0.f);
}
