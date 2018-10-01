/************************************************************************/
/* File: AnimationFrame.hpp
/* Author: Andrew Chase
/* Date: October 1st, 2018
/* Description: Class to represent a single frame of an animation
/************************************************************************/
#pragma once
#include <string>

enum eFrameDirection
{
	DIRECTION_EAST = 0,
	DIRECTION_NORTH,
	DIRECTION_WEST,
	DIRECTION_SOUTH,
	NUM_DIRECTIONS
};

class Texture3D;

class VoxelSprite
{
public:
	//-----Public Methods-----
	
	VoxelSprite(const std::string& name, Texture3D* east, Texture3D* north, Texture3D* west, Texture3D* south);

	
public:
	//-----Public Data-----
	

private:
	//-----Private Methods-----
	
	
private:
	//-----Private Data-----
	
	std::string		m_name;
	Texture3D*		m_textures[NUM_DIRECTIONS];
	
};
