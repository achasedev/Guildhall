#include "Engine/Core/Rgba.hpp"
#include "Game/Framework/GameCommon.hpp"
#include "Game/Framework/VoxelTerrain.hpp"
#include "Engine/Rendering/Resources/VoxelTexture.hpp"

std::map<std::string, VoxelTerrain*> VoxelTerrain::s_terrains;

const IntVector3 VoxelTerrain::TERRAIN_DIMENSIONS = IntVector3(256, 64, 256);

VoxelTerrain* VoxelTerrain::Clone() const
{
	VoxelTerrain* clone = new VoxelTerrain();
	clone->m_name = m_name;
	clone->m_texture = m_texture->Clone();

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

