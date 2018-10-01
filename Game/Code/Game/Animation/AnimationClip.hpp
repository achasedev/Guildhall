/************************************************************************/
/* File: AnimationClip.hpp
/* Author: Andrew Chase
/* Date: October 1st, 2018
/* Description: Class to represent animation data for a single animation
/************************************************************************/
#pragma once
#include <string>

class Texture3D;

enum ePlayMode
{
	PLAYMODE_DEFAULT,
	PLAYMODE_CLAMP,
	PLAYMODE_LOOP,
	NUM_PLAYMODES
};

struct AnimFrame
{
	Texture3D* m_texture = nullptr;

};

class AnimationClip
{
public:
	//-----Public Methods-----
	
	bool LoadFromFile(const char* filename);

public:
	//-----Public Data-----
	

private:
	//-----Private Methods-----
	
	
private:
	//-----Private Data-----
	
	std::string m_name;
	ePlayMode	m_playMode = PLAYMODE_DEFAULT;

	Texture3D* m_sprites = nullptr;

};
