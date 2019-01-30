#include "Engine/Core/Rgba.hpp"
#include "Game/Framework/GameCommon.hpp"
#include "Game/Animation/VoxelSprite.hpp"
#include "Game/Animation/VoxelSprite.hpp"
#include "Game/Framework/VoxelMap.hpp"
#include "Game/Entity/EntityDefinition.hpp"
#include "Game/Framework/MapDefinition.hpp"
#include "Game/Framework/VoxelTerrain.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Utility/HeatMap.hpp"
#include "Engine/Core/Utility/XmlUtilities.hpp"

const IntVector3 VoxelMap::MAP_DIMENSIONS = IntVector3(256, 64, 256);


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


void VoxelMap::AddVoxel(const IntVector3& relativeCoords, const Rgba& color)
{
	m_terrain->SetColorAtRelativeCoords(relativeCoords, 0.f, color);

	// Update the height map if applicable
	int oldHeight = (int)m_heightmap->GetHeat(relativeCoords.xz());
	if (relativeCoords.y == oldHeight)
	{
		m_heightmap->SetHeat(relativeCoords.xz(), (float)(oldHeight + 1));
	}
}

Rgba VoxelMap::RemoveVoxel(const IntVector3& relativeCoords)
{
	Rgba color = m_terrain->GetColorAtRelativeCoords(relativeCoords, 0.f);
	m_terrain->SetColorAtRelativeCoords(relativeCoords, 0.f, Rgba(0, 0, 0, 0));

	// Update the height map if applicable
	int oldHeight = (int)m_heightmap->GetHeat(relativeCoords.xz());
	if (relativeCoords.y == oldHeight - 1)
	{
		m_heightmap->SetHeat(relativeCoords.xz(), (float)(oldHeight - 1));
	}

	return color;
}

int VoxelMap::GetHeightAtCoords(const IntVector2& coords)
{
	return (int)m_heightmap->GetHeat(coords);
}

Rgba VoxelMap::GetColorAtCoords(const IntVector3& coords)
{
	return m_terrain->GetColorAtRelativeCoords(coords, 0.f);
}
