#include "Engine/Core/Rgba.hpp"
#include "Game/Framework/GameCommon.hpp"
#include "Game/Animation/VoxelSprite.hpp"
#include "Game/Framework/VoxelTerrain.hpp"
#include "Game/Entity/EntityDefinition.hpp"
#include "Engine/Rendering/Resources/VoxelTexture.hpp"

std::map<std::string, VoxelTerrain*> VoxelTerrain::s_terrains;

const IntVector3 VoxelTerrain::TERRAIN_DIMENSIONS = IntVector3(256, 64, 256);

VoxelTerrain* VoxelTerrain::Clone() const
{
	VoxelTerrain* clone = new VoxelTerrain();
	clone->m_name = m_name;
	clone->m_texture = m_texture->Clone();
	clone->m_initialEntities = m_initialEntities;

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

void VoxelTerrain::LoadTerrain(const XMLElement& terrainElement)
{
	std::string name = ParseXmlAttribute(terrainElement, "name", "");
	bool nameAlreadyTaken = s_terrains.find(name) != s_terrains.end();

	ASSERT_OR_DIE(!nameAlreadyTaken, Stringf("Error: Duplicate terrain name \"%s\"", name.c_str()));

	std::string filepath = ParseXmlAttribute(terrainElement, "file", "");

	VoxelTexture* texture = new VoxelTexture();
	bool success = texture->CreateFromFile(filepath.c_str(), false);
	ASSERT_OR_DIE(success, Stringf("Error: Couldn't load terrain \"%s\"", name.c_str()));

	VoxelTerrain* terrain = new VoxelTerrain();
	terrain->m_texture = texture;
	terrain->m_name = name;

	// Post process - remove chroma keys
	for (int y = 0; y < TERRAIN_DIMENSIONS.y; ++y)
	{
		for (int z = 0; z < TERRAIN_DIMENSIONS.z; ++z)
		{
			for (int x = 0; x < TERRAIN_DIMENSIONS.x; ++x)
			{
				IntVector3 coord = IntVector3(x, y, z);
				Rgba color = texture->GetColorAtCoords(coord);

				if (color.r == 255)
				{
					// Remove the key
					texture->SetColorAtCoords(coord, Rgba(0, 0, 0, 0));

					// Add the entity by ID
					EntitySpawn_t spawn;

					// Center the spawn
					spawn.definition = EntityDefinition::GetDefinition((int)color.g);

					IntVector3 halfDimensions = spawn.definition->GetDefaultSprite()->GetDimensions() / 2;

					spawn.position = Vector3(IntVector3(coord.x - halfDimensions.x, coord.y, coord.z - halfDimensions.z));
					spawn.orientation = (float)color.b * 2.f;

					terrain->m_initialEntities.push_back(spawn);
				}
			}
		}
	}

	// Add it to the registry of prototypes
	s_terrains[name] = terrain;
}


void VoxelTerrain::AddVoxel(const IntVector3& coords, const Rgba& color)
{
	m_texture->SetColorAtCoords(coords, color);
}

Rgba VoxelTerrain::RemoveVoxel(const IntVector3& coords)
{
	Rgba color = m_texture->GetColorAtCoords(coords);
	m_texture->SetColorAtCoords(coords, Rgba(0, 0, 0, 0));

	return color;
}

int VoxelTerrain::GetHeightAtCoords(const IntVector2& coords)
{
	for (int y = 0; y < TERRAIN_DIMENSIONS.y; ++y)
	{
		Rgba color = m_texture->GetColorAtCoords(IntVector3(coords.x, y, coords.y));
		if (color.a == 0)
		{
			return y;
		}
	}

	return 0;
}

Rgba VoxelTerrain::GetColorAtCoords(const IntVector3& coords)
{
	return m_texture->GetColorAtCoords(coords);
}


//-----------------------------------------------------------------------------------------------
// Returns the list of entities that are initially spawned on this terrain
//
const std::vector<EntitySpawn_t>& VoxelTerrain::GetInitialEntities() const
{
	return m_initialEntities;
}

