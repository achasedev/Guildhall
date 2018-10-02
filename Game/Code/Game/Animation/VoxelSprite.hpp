/************************************************************************/
/* File: AnimationFrame.hpp
/* Author: Andrew Chase
/* Date: October 1st, 2018
/* Description: Class to represent a single frame of an animation
/************************************************************************/
#pragma once
#include <map>
#include <string>

enum eFrameDirection
{
	DIRECTION_EAST = 0,
	DIRECTION_NORTH,
	DIRECTION_WEST,
	DIRECTION_SOUTH,
	NUM_DIRECTIONS
};

class VoxelTexture;

class VoxelSprite
{
public:
	//-----Public Methods-----
	
	VoxelSprite(const std::string& name, VoxelTexture* east, VoxelTexture* north, VoxelTexture* west, VoxelTexture* south);

	VoxelTexture* GetTextureForOrientation(float angle);

	// Statics
	static void					LoadVoxelSpriteFromFile(const std::string& filename);
	static const VoxelSprite*	GetVoxelSprite(const std::string& name);
	

public:
	//-----Public Data-----
	

private:
	//-----Private Methods-----
	
	VoxelSprite(const VoxelSprite& copy) = delete;

	
private:
	//-----Private Data-----
	
	std::string		m_name;
	VoxelTexture*	m_textures[NUM_DIRECTIONS];

	static std::map<std::string, const VoxelSprite*> s_sprites;
	
};
