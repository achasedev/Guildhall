/************************************************************************/
/* File: AnimationClip.hpp
/* Author: Andrew Chase
/* Date: October 1st, 2018
/* Description: Class to represent animation data for a single animation
/************************************************************************/
#pragma once
#include <map>
#include <string>
#include "Engine/Core/Utility/XmlUtilities.hpp"

class Texture3D;
class VoxelSprite;

enum ePlayMode
{
	PLAYMODE_DEFAULT,
	PLAYMODE_CLAMP,
	PLAYMODE_LOOP,
	NUM_PLAYMODES
};

struct VoxelAnimFrame
{
	VoxelAnimFrame(VoxelSprite* _sprite, float _duration)
	 : sprite(_sprite), duration(_duration) {}

	VoxelSprite*	sprite = nullptr;
	float			duration = 0.f;
};

class VoxelAnimation
{
public:
	//-----Public Methods-----
	

	std::string		GetName() const;
	VoxelAnimation*	CloneAnimationClip(const std::string& name);


public:
	//-----Public Data-----
	

private:
	//-----Private Methods-----
	
	VoxelAnimation(const XMLElement& animElement);
	

private:
	//-----Private Data-----
	
	std::string			m_name;
	ePlayMode			m_playMode = PLAYMODE_DEFAULT;
	VoxelAnimFrame*		m_frames = nullptr;

	static std::map<std::string, VoxelAnimation*> s_clipPrototypes;

};
