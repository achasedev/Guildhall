#include "Game/Animation/VoxelSprite.hpp"
#include "Engine/Assets/AssetDB.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Utility/StringUtils.hpp"
#include "Engine/Core/Utility/ErrorWarningAssert.hpp"
#include "Engine/Rendering/Resources/VoxelTexture.hpp"

std::map<std::string, const VoxelSprite*> VoxelSprite::s_sprites;

VoxelSprite::VoxelSprite(const std::string& name, VoxelTexture* east, VoxelTexture* north, VoxelTexture* west, VoxelTexture* south)
{
	m_name = name;
	m_textures[DIRECTION_EAST] = east;
	m_textures[DIRECTION_NORTH] = north;
	m_textures[DIRECTION_WEST] = west;
	m_textures[DIRECTION_SOUTH] = south;
}

VoxelTexture* VoxelSprite::GetTextureForOrientation(float angle)
{
	float cardinalAngle = GetNearestCardinalAngle(angle);

	if		(cardinalAngle == 0.f)		{ return m_textures[DIRECTION_EAST]; }
	else if (cardinalAngle == 90.f)		{ return m_textures[DIRECTION_NORTH]; }
	else if (cardinalAngle == 180.f)	{ return m_textures[DIRECTION_WEST]; }
	else								{ return m_textures[DIRECTION_SOUTH]; }
}

void VoxelSprite::LoadVoxelSpriteFromFile(const std::string& filename)
{
	VoxelTexture* northTexture = AssetDB::CreateOrGet3DVoxelTextureInstance(filename);

	if (northTexture == nullptr)
	{
		return;
	}

	IntVector3 dimensions = northTexture->GetDimensions();

	// Rotate to get the other 3
	dimensions = northTexture->GetDimensions();

	ASSERT_OR_DIE(dimensions.x == dimensions.z, Stringf("Error: VoxelSprite::CreateOrGetVoxelSprite() had a texture with unequal xz dimensions, file was %s", filename));

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
}

const VoxelSprite* VoxelSprite::GetVoxelSprite(const std::string& name)
{
	bool spriteExists = s_sprites.find(name) != s_sprites.end();

	if (spriteExists)
	{
		return s_sprites.at(name);
	}
	
	return nullptr;
}
