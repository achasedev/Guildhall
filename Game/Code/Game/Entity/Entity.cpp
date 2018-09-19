#include "Game/Entity/Entity.hpp"
#include "Engine/Core/Rgba.hpp"
#include "Engine/Assets/AssetDB.hpp"
#include <stdlib.h>

#define MAX_ENTITY_VOXEL_COUNT 8*8*8

Entity::Entity()
{
	m_texture = AssetDB::CreateOrGet3DVoxelTextureInstance("Data/3DTextures/Test.qef");
}

Entity::~Entity()
{
	if (m_texture != nullptr)
	{
		free(m_texture);
		m_texture = nullptr;
	}
}

void Entity::Update()
{
}

Texture3D* Entity::GetTexture() const
{
	return m_texture;
}
