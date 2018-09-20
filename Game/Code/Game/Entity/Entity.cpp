#include "Game/Entity/Entity.hpp"
#include "Engine/Core/Rgba.hpp"
#include "Engine/Assets/AssetDB.hpp"
#include "Engine/Math/MathUtils.hpp"

#include <stdlib.h>

#define MAX_ENTITY_VOXEL_COUNT 8*8*8

Entity::Entity()
{
	m_textures[0] = AssetDB::CreateOrGet3DVoxelTextureInstance("Data/3DTextures/Test.qef");

	// Rotate to get the other 3
	IntVector3 dimensions = m_textures[0]->GetDimensions();
	unsigned int voxelCount = m_textures[0]->GetVoxelCount();

	// South
	m_textures[1] = m_textures[0]->Copy();
	Rgba* colorData = m_textures[1]->GetColorData();

	for (int height = 0; height < dimensions.y; ++height)
	{
		for (int row = 0; row < dimensions.z; ++row)
		{
			for (int col = 0; col < dimensions.x; ++col)
			{
				int myZ = dimensions.z - row;
				int index = height * (dimensions.x * dimensions.z) + row * dimensions.x + col;
				colorData[index] = m_textures[0]->GetColorAtCoords(IntVector3(col, height, myZ));
			}
		}
	}


	// For testing
	m_textures[0] = m_textures[1];

	// East

	// West
}

Entity::~Entity()
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

void Entity::Update()
{
}

Texture3D* Entity::GetTexture() const
{

	return m_textures[0];
}
