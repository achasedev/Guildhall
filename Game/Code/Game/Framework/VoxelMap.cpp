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
std::map<std::string, VoxelMap*> VoxelMap::s_maps;

const IntVector3 VoxelMap::MAP_DIMENSIONS = IntVector3(256, 64, 256);

VoxelMap* VoxelMap::Clone() const
{
	VoxelMap* clone = new VoxelMap();
	clone->m_name = m_name;
	clone->m_terrain = m_terrain->Clone();
	clone->m_staticSpawnAreas = m_staticSpawnAreas;
	clone->m_heightmap = new HeatMap(*m_heightmap);

	return clone;
}

VoxelMap* VoxelMap::GetMapClone(const std::string& mapName)
{
	bool nameExists = s_maps.find(mapName) != s_maps.end();

	if (nameExists)
	{
		return s_maps.at(mapName)->Clone();
	}

	return nullptr;
}

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

#include "Engine/Core/DeveloperConsole/DevConsole.hpp"
void VoxelMap::LoadMap(const std::string& mapFilePath)
{
	// Load the XML document
	XMLDocument document;
	XML_ERROR error = document.LoadFile(mapFilePath.c_str());

	if (error != tinyxml2::XML_SUCCESS)
	{
		ConsoleErrorf("Couldn't open map file %s", mapFilePath.c_str());
		return;
	}

	const XMLElement* root = document.RootElement();

	// Name
	std::string name = ParseXmlAttribute(*root, "name", name);

	// Terrain File
	VoxelSprite* terrainSprite = new VoxelSprite();
	bool spriteLoaded = terrainSprite->CreateFromFile()

	std::string name = ParseXmlAttribute(mapElement, "name", "");
	bool nameAlreadyTaken = s_maps.find(name) != s_maps.end();

	ASSERT_OR_DIE(!nameAlreadyTaken, Stringf("Error: Duplicate map name \"%s\"", name.c_str()));

	std::string filepath = ParseXmlAttribute(mapElement, "file", "");

	VoxelSprite* texture = new VoxelSprite();
	bool success = texture->CreateFromFile(filepath.c_str(), false);
	ASSERT_OR_DIE(success, Stringf("Error: Couldn't load map \"%s\"", name.c_str()));

	HeatMap* heightMap = new HeatMap(MAP_DIMENSIONS.xz(), 0.f);

	for (int z = 0; z < MAP_DIMENSIONS.z; ++z)
	{
		for (int x = 0; x < MAP_DIMENSIONS.x; ++x)
		{
			IntVector2 coords = IntVector2(x, z);
			heightMap->SetHeat(coords, (float) GetSpriteHeightAtCoords(texture, coords, MAP_DIMENSIONS));
		}
	}
	
	VoxelMap* map = new VoxelMap();
	map->m_terrain = texture;
	map->m_name = name;
	map->m_heightmap = heightMap;
	

	// Add it to the registry of prototypes
	s_maps[name] = map;
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


//-----------------------------------------------------------------------------------------------
// Returns the list of entities that are initially spawned on this map
//
const std::vector<EntitySpawn_t>& VoxelMap::GetInitialEntities() const
{
	return m_staticSpawnAreas;
}

