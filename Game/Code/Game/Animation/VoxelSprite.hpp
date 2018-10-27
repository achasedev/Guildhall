/************************************************************************/
/* File: AnimationFrame.hpp
/* Author: Andrew Chase
/* Date: October 1st, 2018
/* Description: Class to represent a single frame of an animation
/************************************************************************/
#pragma once
#include <map>
#include <string>
#include "Engine/Math/IntVector3.hpp"
#include "Engine/Core/Utility/XmlUtilities.hpp"

// For directionality
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

	VoxelSprite(const std::string& name, const std::string& filename);
	~VoxelSprite();

	// Accessors
	const VoxelTexture*			GetTextureForOrientation(float angle) const;

	// Producers
	IntVector3					GetDimensions() const;
	VoxelSprite*				Clone() const;

	// Statics
	static void					LoadVoxelSprites(const std::string& filename);
	static VoxelSprite*			CreateVoxelSpriteClone(const std::string& name);
	static const VoxelSprite*	GetVoxelSprite(const std::string& name);


private:
	//-----Private Methods-----

	// No copying for now
	VoxelSprite(const VoxelSprite& copy);

	
private:
	//-----Private Data-----
	
	std::string		m_name;
	IntVector3		m_dimensions;
	VoxelTexture*	m_textures[NUM_DIRECTIONS];

	// All sprites in the game are stored here
	static std::map<std::string, const VoxelSprite*> s_sprites;
	
};
