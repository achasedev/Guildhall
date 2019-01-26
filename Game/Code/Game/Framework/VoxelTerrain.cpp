#include "Engine/Core/Rgba.hpp"
#include "Game/Framework/GameCommon.hpp"
#include "Game/Animation/VoxelSprite.hpp"
#include "Game/Animation/VoxelSprite.hpp"
#include "Game/Framework/VoxelTerrain.hpp"
#include "Game/Entity/EntityDefinition.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Utility/HeatMap.hpp"

std::map<std::string, VoxelTerrain*> VoxelTerrain::s_terrains;

const IntVector3 VoxelTerrain::TERRAIN_DIMENSIONS = IntVector3(256, 64, 256);

VoxelTerrain* VoxelTerrain::Clone() const
{
	VoxelTerrain* clone = new VoxelTerrain();
	clone->m_name = m_name;
	clone->m_texture = m_texture->Clone();
	clone->m_initialEntities = m_initialEntities;
	clone->m_heightmap = new HeatMap(*m_heightmap);

	return clone;
}

VoxelTerrain* VoxelTerrain::GetTerrainClone(const std::string& terrainName)
{
	bool nameExists = s_terrains.find(terrainName) != s_terrains.end();

	if (nameExists)
	{
		return s_terrains.at(terrainName)->Clone();
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

	return 0;
}

void VoxelTerrain::LoadTerrain(const XMLElement& terrainElement)
{
	std::string name = ParseXmlAttribute(terrainElement, "name", "");
	bool nameAlreadyTaken = s_terrains.find(name) != s_terrains.end();

	ASSERT_OR_DIE(!nameAlreadyTaken, Stringf("Error: Duplicate terrain name \"%s\"", name.c_str()));

	std::string filepath = ParseXmlAttribute(terrainElement, "file", "");

	VoxelSprite* texture = new VoxelSprite();
	bool success = texture->CreateFromFile(filepath.c_str(), false);
	ASSERT_OR_DIE(success, Stringf("Error: Couldn't load terrain \"%s\"", name.c_str()));

	HeatMap* heightMap = new HeatMap(TERRAIN_DIMENSIONS.xz(), 0.f);

	for (int z = 0; z < TERRAIN_DIMENSIONS.z; ++z)
	{
		for (int x = 0; x < TERRAIN_DIMENSIONS.x; ++x)
		{
			IntVector2 coords = IntVector2(x, z);
			heightMap->SetHeat(coords, (float) GetSpriteHeightAtCoords(texture, coords, TERRAIN_DIMENSIONS));
		}
	}
	
	VoxelTerrain* terrain = new VoxelTerrain();
	terrain->m_texture = texture;
	terrain->m_name = name;
	terrain->m_heightmap = heightMap;
	
	// Post process - remove chroma keys
	for (int y = 0; y < TERRAIN_DIMENSIONS.y; ++y)
	{
		for (int z = 0; z < TERRAIN_DIMENSIONS.z; ++z)
		{
			for (int x = 0; x < TERRAIN_DIMENSIONS.x; ++x)
			{
				IntVector3 coord = IntVector3(x, y, z);
				Rgba color = texture->GetColorAtRelativeCoords(coord, 0.f);

				if (color.r == 255)
				{
					// Remove the key
					texture->SetColorAtRelativeCoords(coord, 0.f, Rgba(0, 0, 0, 0));

					// Add the entity by ID
					EntitySpawn_t spawn;

					// Center the spawn
					spawn.definition = EntityDefinition::GetDefinition((int)color.g);

					IntVector3 halfDimensions = spawn.definition->GetDefaultSprite()->GetBaseDimensions() / 2;

					spawn.position = Vector3(IntVector3(coord.x, coord.y, coord.z));
					spawn.orientation = 180.f;

					terrain->m_initialEntities.push_back(spawn);
				}
			}
		}
	}

	// Add it to the registry of prototypes
	s_terrains[name] = terrain;
}


void VoxelTerrain::AddVoxel(const IntVector3& relativeCoords, const Rgba& color)
{
	m_texture->SetColorAtRelativeCoords(relativeCoords, 0.f, color);

	// Update the height map if applicable
	int oldHeight = (int)m_heightmap->GetHeat(relativeCoords.xz());
	if (relativeCoords.y == oldHeight)
	{
		m_heightmap->SetHeat(relativeCoords.xz(), (float)(oldHeight + 1));
	}
}

Rgba VoxelTerrain::RemoveVoxel(const IntVector3& relativeCoords)
{
	Rgba color = m_texture->GetColorAtRelativeCoords(relativeCoords, 0.f);
	m_texture->SetColorAtRelativeCoords(relativeCoords, 0.f, Rgba(0, 0, 0, 0));

	// Update the height map if applicable
	int oldHeight = (int)m_heightmap->GetHeat(relativeCoords.xz());
	if (relativeCoords.y == oldHeight - 1)
	{
		m_heightmap->SetHeat(relativeCoords.xz(), (float)(oldHeight - 1));
	}

	return color;
}

int VoxelTerrain::GetHeightAtCoords(const IntVector2& coords)
{
	return (int)m_heightmap->GetHeat(coords);
}

Rgba VoxelTerrain::GetColorAtCoords(const IntVector3& coords)
{
	return m_texture->GetColorAtRelativeCoords(coords, 0.f);
}


//-----------------------------------------------------------------------------------------------
// Returns the list of entities that are initially spawned on this terrain
//
const std::vector<EntitySpawn_t>& VoxelTerrain::GetInitialEntities() const
{
	return m_initialEntities;
}

