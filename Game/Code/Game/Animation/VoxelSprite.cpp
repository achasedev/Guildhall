#include "Game/Animation/VoxelSprite.hpp"
#include "Engine/Rendering/Resources/VoxelTexture.hpp"

VoxelSprite::VoxelSprite(const std::string& name, VoxelTexture* east, VoxelTexture* north, VoxelTexture* west, VoxelTexture* south)
{
	m_name = name;
	m_textures[DIRECTION_EAST] = east;
	m_textures[DIRECTION_NORTH] = north;
	m_textures[DIRECTION_WEST] = west;
	m_textures[DIRECTION_SOUTH] = south;
}
