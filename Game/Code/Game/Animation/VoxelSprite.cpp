/************************************************************************/
/* File: VoxelSprite.cpp
/* Author: Andrew Chase
/* Date: October 2nd, 2018
/* Description: Implementation of the VoxelSprite class
/************************************************************************/
#include "Game/Animation/VoxelSprite.hpp"
#include "Engine/Assets/AssetDB.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Utility/StringUtils.hpp"
#include "Engine/Core/Utility/ErrorWarningAssert.hpp"
#include "Engine/Rendering/Resources/VoxelTexture.hpp"

// Global map of sprites
std::map<std::string, const VoxelSprite*> VoxelSprite::s_sprites;


//-----------------------------------------------------------------------------------------------
// Constructor - takes the file name of the xml file
//
VoxelSprite::VoxelSprite(const std::string& name, const std::string& filename)
	: m_name(name)
{
	VoxelTexture* northTexture = AssetDB::CreateOrGetVoxelTexture(filename);
	//ASSERT_OR_DIE(northTexture != nullptr, Stringf("Error: VoxelSprite::VoxelSprite() couldn't open file %s", filename.c_str()));

	// Dimensions check, so we can rotate and have it work
	IntVector3 dimensions = northTexture->GetDimensions();
	//ASSERT_OR_DIE(dimensions.x == dimensions.z, Stringf("Error: VoxelSprite::VoxelSprite() had a texture with unequal xz dimensions, file was %s", filename.c_str()));

	m_dimensions = northTexture->GetDimensions();
	//ASSERT_OR_DIE(m_dimensions.x == m_dimensions.z, "Error: VoxelSprite::VoxelSprite() has unequeal xz dimensions");

	// Rotate to get the other 3 directions if we can
	if (m_dimensions.x == m_dimensions.z)
	{
		// South
		int destIndex = 0;
		VoxelTexture* southTexture = northTexture->Clone();
		for (int y = 0; y < dimensions.y; ++y)
		{
			for (int z = dimensions.z - 1; z >= 0; --z)
			{
				for (int x = dimensions.x - 1; x >= 0; --x)
				{
					int sourceIndex = y * (dimensions.x * dimensions.z) + z * dimensions.x + x;
					southTexture->SetColorAtIndex(destIndex, northTexture->GetColorAtIndex(sourceIndex));
					destIndex++;
				}
			}
		}

		// East
		VoxelTexture* eastTexture = northTexture->Clone();
		destIndex = 0;
		for (int y = 0; y < dimensions.y; ++y)
		{
			for (int x = dimensions.x - 1; x >= 0; --x)
			{
				for (int z = 0; z < dimensions.z; ++z)
				{
					int sourceIndex = y * (dimensions.x * dimensions.z) + z * dimensions.x + x;
					eastTexture->SetColorAtIndex(destIndex, northTexture->GetColorAtIndex(sourceIndex));
					destIndex++;
				}
			}
		}

		// West
		VoxelTexture* westTexture = northTexture->Clone();
		destIndex = 0;
		for (int y = 0; y < dimensions.y; ++y)
		{
			for (int x = 0; x < dimensions.x; ++x)
			{
				for (int z = dimensions.z - 1; z >= 0; --z)
				{
					int sourceIndex = y * (dimensions.x * dimensions.z) + z * dimensions.x + x;
					westTexture->SetColorAtIndex(destIndex, northTexture->GetColorAtIndex(sourceIndex));
					destIndex++;
				}
			}
		}

		// Set the textures
		m_textures[DIRECTION_EAST] = eastTexture;
		m_textures[DIRECTION_NORTH] = northTexture;
		m_textures[DIRECTION_WEST] = westTexture;
		m_textures[DIRECTION_SOUTH] = southTexture;
	}
	else
	{
		m_textures[DIRECTION_NORTH] = northTexture;
		m_textures[DIRECTION_EAST] = northTexture->Clone();
		m_textures[DIRECTION_SOUTH] = northTexture->Clone();
		m_textures[DIRECTION_WEST] = northTexture->Clone();
	}
}


//-----------------------------------------------------------------------------------------------
// Returns the texture representing the direction to render for the given angle
//
const VoxelTexture* VoxelSprite::GetTextureForOrientation(float angle) const
{
	float cardinalAngle = GetNearestCardinalAngle(angle);

	if		(cardinalAngle == 0.f)		{ return m_textures[DIRECTION_EAST]; }
	else if (cardinalAngle == 90.f)		{ return m_textures[DIRECTION_NORTH]; }
	else if (cardinalAngle == 180.f)	{ return m_textures[DIRECTION_WEST]; }
	else								{ return m_textures[DIRECTION_SOUTH]; }
}


//-----------------------------------------------------------------------------------------------
// Returns the dimensions of the voxel sprite
//
IntVector3 VoxelSprite::GetDimensions() const
{
	return m_dimensions;
}


//-----------------------------------------------------------------------------------------------
// Loads a VoxelSprite xml file given the filename and constructs the sprites
//
void VoxelSprite::LoadVoxelSprites(const std::string& filename)
{
	// Load the document
	XMLDocument document;
	XMLError error = document.LoadFile(filename.c_str());
	ASSERT_OR_DIE(error == tinyxml2::XML_SUCCESS, Stringf("Error: VoxelSprite::LoadVoxelSpriteFromFile() couldn't load file %s", filename.c_str()));

	// Get the texture file and the name out of the xml document
	const XMLElement* rootElement = document.RootElement();
	ASSERT_OR_DIE(rootElement != nullptr, Stringf("Error: VoxelSprite::LoadVoxelSpriteFromFile() loaded file with no root element: %s", filename.c_str()));

	const XMLElement* spriteElement = rootElement->FirstChildElement();
	while (spriteElement != nullptr)
	{
		// Get the sprite name
		std::string spriteName = ParseXmlAttribute(*spriteElement, "name");
		ASSERT_OR_DIE(spriteName.size() > 0, Stringf("Error: VoxelSprite::LoadVoxelSpritesFromFile() found sprite with no name in %s", filename.c_str()));

		// Get the sprite texture file
		std::string fileName = ParseXmlAttribute(*spriteElement, "file");
		ASSERT_OR_DIE(fileName.size() > 0, Stringf("Error: VoxelSprite::LoadVoxelSpritesFromFile() found sprite with no file in %s", filename.c_str()));

		// Construct the sprite
		VoxelSprite* sprite = new VoxelSprite(spriteName, fileName);
		s_sprites[spriteName] = sprite;

		// Move to the next element
		spriteElement = spriteElement->NextSiblingElement();
	}
}


//-----------------------------------------------------------------------------------------------
// Returns the VoxelSprite given by name if it exists, nullptr otherwise
//
const VoxelSprite* VoxelSprite::GetVoxelSprite(const std::string& name)
{
	bool spriteExists = s_sprites.find(name) != s_sprites.end();

	if (spriteExists)
	{
		return s_sprites.at(name);
	}
	
	return nullptr;
}
