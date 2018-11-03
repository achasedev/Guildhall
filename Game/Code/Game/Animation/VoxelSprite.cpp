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
	VoxelTexture* northTexture = new VoxelTexture();
	bool success = northTexture->CreateFromFile(filename.c_str(), true);

	ASSERT_OR_DIE(success, Stringf("Error: VoxelSprite::VoxelSprite() couldn't open file %s", filename.c_str()));

	// Dimensions check, so we can rotate and have it work
	m_dimensions = northTexture->GetDimensions();


	// Rotate to get the other 3 directions if we can
	if (m_dimensions.x == m_dimensions.z)
	{
		// South
		int destIndex = 0;
		VoxelTexture* southTexture = northTexture->Clone();
		for (int y = 0; y < m_dimensions.y; ++y)
		{
			for (int z = m_dimensions.z - 1; z >= 0; --z)
			{
				for (int x = m_dimensions.x - 1; x >= 0; --x)
				{
					int sourceIndex = y * (m_dimensions.x * m_dimensions.z) + z * m_dimensions.x + x;
					southTexture->SetColorAtIndex(destIndex, northTexture->GetColorAtIndex(sourceIndex));
					destIndex++;
				}
			}
		}

		// East
		VoxelTexture* eastTexture = northTexture->Clone();
		destIndex = 0;
		for (int y = 0; y < m_dimensions.y; ++y)
		{
			for (int x = m_dimensions.x - 1; x >= 0; --x)
			{
				for (int z = 0; z < m_dimensions.z; ++z)
				{
					int sourceIndex = y * (m_dimensions.x * m_dimensions.z) + z * m_dimensions.x + x;
					eastTexture->SetColorAtIndex(destIndex, northTexture->GetColorAtIndex(sourceIndex));
					destIndex++;
				}
			}
		}

		// West
		VoxelTexture* westTexture = northTexture->Clone();
		destIndex = 0;
		for (int y = 0; y < m_dimensions.y; ++y)
		{
			for (int x = 0; x < m_dimensions.x; ++x)
			{
				for (int z = m_dimensions.z - 1; z >= 0; --z)
				{
					int sourceIndex = y * (m_dimensions.x * m_dimensions.z) + z * m_dimensions.x + x;
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
		m_textures[DIRECTION_EAST] = nullptr;
		m_textures[DIRECTION_SOUTH] = nullptr;
		m_textures[DIRECTION_WEST] = nullptr;
	}
}


//-----------------------------------------------------------------------------------------------
// Private copy constructor
//
VoxelSprite::VoxelSprite(const VoxelSprite& copy)
{
	m_name = copy.m_name;
	m_dimensions = copy.m_dimensions;

	for (int i = 0; i < NUM_DIRECTIONS; ++i)
	{
		if (copy.m_textures[i] != nullptr)
		{
			m_textures[i] = copy.m_textures[i]->Clone();
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Destructor
//
VoxelSprite::~VoxelSprite()
{
	for (int i = 0; i < NUM_DIRECTIONS; ++i)
	{
		if (m_textures[i] != nullptr)
		{
			delete m_textures[i];
			m_textures[i] = nullptr;
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Returns the texture representing the direction to render for the given angle
//
const VoxelTexture* VoxelSprite::GetTextureForOrientation(float angle) const
{
	float cardinalAngle = GetNearestCardinalAngle(angle);

	const VoxelTexture* toReturn = nullptr;

	if (cardinalAngle == 0.f)			{ toReturn = m_textures[DIRECTION_EAST]; }
	else if (cardinalAngle == 90.f)		{ toReturn = m_textures[DIRECTION_NORTH]; }
	else if (cardinalAngle == 180.f)	{ toReturn = m_textures[DIRECTION_WEST]; }
	else								{ toReturn = m_textures[DIRECTION_SOUTH]; }

	if (toReturn == nullptr)
	{
		return m_textures[DIRECTION_NORTH];
	}

	return toReturn;
}


//-----------------------------------------------------------------------------------------------
// Returns the dimensions of the voxel sprite
//
IntVector3 VoxelSprite::GetDimensions() const
{
	return m_dimensions;
}


//-----------------------------------------------------------------------------------------------
// Clones the VoxelSprite (deep copy)
//
VoxelSprite* VoxelSprite::Clone() const
{
	return new VoxelSprite(*this);
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
// Returns a clone of the VoxelSprite given by name if it exists, nullptr otherwise
//
VoxelSprite* VoxelSprite::CreateVoxelSpriteClone(const std::string& name)
{
	const VoxelSprite* baseSprite = GetVoxelSprite(name);

	if (baseSprite != nullptr)
	{
		return baseSprite->Clone();
	}

	return nullptr;
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
