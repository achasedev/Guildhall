#include "Game/Animation/VoxelSprite.hpp"
#include "Game/Framework/VoxelTerrain.hpp"
#include "Engine/Core/Rgba.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Utility/StringUtils.hpp"
#include "Engine/Core/Utility/XmlUtilities.hpp"
#include "Engine/Core/Utility/ErrorWarningAssert.hpp"


void VoxelTerrain::LoadTerrainFile(const std::string& terrainFile)
{
	XMLDocument document;
	XMLError error = document.LoadFile(terrainFile.c_str());
	GUARANTEE_OR_DIE(error == tinyxml2::XML_SUCCESS, Stringf("Couldn't open file %s", terrainFile.c_str()));

	const XMLElement* rootElement = document.RootElement();
	GUARANTEE_OR_DIE(rootElement != nullptr, Stringf("Terrain file %s has no root element", terrainFile.c_str()));

	// Get the type name
	std::string type = ParseXmlAttribute(*rootElement, "type", "");
	GUARANTEE_OR_DIE(!IsStringNullOrEmpty(type), Stringf("Terrain file %shas no type specified"));

	// Vector for storing the names that are associated with this type
	std::vector <std::string>& namesUnderType = s_terrainsByType[type];

	const XMLElement* terrainElement = rootElement->FirstChildElement("VoxelTerrain");
	GUARANTEE_OR_DIE(terrainElement != nullptr, Stringf("Terrain file %s has no terrain elements", terrainFile.c_str()));

	while (terrainElement != nullptr)
	{
		std::string name = ParseXmlAttribute(*terrainElement, "name", "");
		GUARANTEE_OR_DIE(!IsStringNullOrEmpty(name), Stringf("Terrain file %s has a terrain element with no name specified", terrainFile.c_str()));

		std::string voxelModelFilePath = ParseXmlAttribute(*terrainElement, "file", "");
		GUARANTEE_OR_DIE(!IsStringNullOrEmpty(voxelModelFilePath), Stringf("Terrain file %s has a terrain element with no file specified", terrainFile.c_str()));

		// Load the sprite
		VoxelSprite* sprite = new VoxelSprite();
		sprite->m_name = name;
		bool success = sprite->CreateFromFile(voxelModelFilePath.c_str(), false);

		if (!success)
		{
			ERROR_AND_DIE(Stringf("Terrain file %s tried to create sprite %s but failed"), terrainFile.c_str(), voxelModelFilePath.c_str());
		}

		// Add the sprite for use later
		VoxelSprite::AddSpriteToRegistry(sprite);

		// Store the name, check for duplicates
		int currentSize = (int)namesUnderType.size();
		for (int i = 0; i < currentSize; ++i)
		{
			GUARANTEE_OR_DIE(namesUnderType[i] != name, Stringf("Voxel Terrain file %s dupicated terrain name \"%s\"", terrainFile.c_str(), name.c_str()));
		}

		namesUnderType.push_back(name);

		terrainElement = terrainElement->NextSiblingElement("VoxelTerrain");
	}
}


VoxelTerrain* VoxelTerrain::CreateVoxelTerrainCloneForName(const std::string& name)
{
	const VoxelSprite* sprite = VoxelSprite::GetVoxelSprite(name);

	if (sprite != nullptr)
	{
		VoxelTerrain* terrain = new VoxelTerrain();

		terrain->m_name = sprite->m_name;
		terrain->m_dimensions = sprite->m_dimensions;

		int voxelCount = terrain->m_dimensions.x * terrain->m_dimensions.y * terrain->m_dimensions.z;
		size_t byteSize = sizeof(Rgba) * voxelCount;
		terrain->m_colorData = (Rgba*)malloc(byteSize);

		memcpy(terrain->m_colorData, sprite->m_colorData, byteSize);

		// Get the type
		terrain->m_terrainType = FindTypeForTerrain(terrain);

		return terrain;
	}

	return nullptr;
}

VoxelTerrain* VoxelTerrain::CreateVoxelTerrainCloneForType(const std::string& type)
{
	// Get a random name
	const std::vector<std::string>& namesUnderType = s_terrainsByType[type];

	int size = (int)namesUnderType.size();
	int randomIndex = GetRandomIntLessThan(size);
	
	return CreateVoxelTerrainCloneForName(namesUnderType[randomIndex]);
}

std::string VoxelTerrain::FindTypeForTerrain(VoxelTerrain* terrain)
{
	std::map < std::string, std::vector<std::string>>::const_iterator itr = s_terrainsByType.begin();

	for (itr; itr != s_terrainsByType.end(); itr++)
	{
		const std::vector<std::string>& namesUnderType = itr->second;

		for (int nameIndex = 0; nameIndex < (int)namesUnderType.size(); ++nameIndex)
		{
			if (namesUnderType[nameIndex] == terrain->m_name)
			{
				terrain->m_terrainType = itr->first;
				return;
			}
		}
	}
}

