/************************************************************************/
/* File: AnimationClip.hpp
/* Author: Andrew Chase
/* Date: October 1st, 2018
/* Description: Class to represent animation data for a single animation
/************************************************************************/
#pragma once
#include "Game/Animation/VoxelSprite.hpp"
#include "Engine/Core/Utility/XmlUtilities.hpp"
#include <map>
#include <vector>
#include <string>

class VoxelTexture;

enum ePlayMode
{
	PLAYMODE_DEFAULT,
	PLAYMODE_CLAMP,
	PLAYMODE_LOOP,
	NUM_PLAYMODES
};

struct VoxelAnimFrame
{
	VoxelAnimFrame(const VoxelSprite* _sprite, float _duration)
	 : sprite(_sprite), duration(_duration) {}

	const VoxelSprite*	sprite = nullptr;
	float				duration = 0.f;
};

class VoxelAnimation
{
public:
	//-----Public Methods-----
	
	// Accessors
	std::string		GetName() const;

	// Producers
	const VoxelSprite*	Evaluate(float timeIntoAnimation, ePlayMode modeOverride) const;
	float				GetTotalDuration() const;

	// Statics
	static void						LoadVoxelAnimations(const std::string& filename);
	static const VoxelAnimation*	GetAnimationClip(const std::string& name);


private:
	//-----Private Methods-----
	
	VoxelAnimation(const XMLElement& animElement);
	VoxelAnimation(const VoxelAnimation& copy) = delete;


private:
	//-----Private Data-----
	
	std::string						m_name;
	ePlayMode						m_playMode = PLAYMODE_DEFAULT;
	std::vector<VoxelAnimFrame>		m_frames;

	static std::map<std::string, const VoxelAnimation*> s_voxelAnimations;

};
