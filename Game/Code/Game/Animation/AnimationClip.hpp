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
class AnimationFrame;

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
	

	std::string GetName() const;

public:
	//-----Public Data-----
	

private:
	//-----Private Methods-----
	
	AnimationClip(const XMLElement& animElement);
	

private:
	//-----Private Data-----
	
	std::string			m_name;
	ePlayMode			m_playMode = PLAYMODE_DEFAULT;
	AnimationFrame*		m_frames = nullptr;

	static std::map<std::string, AnimationClip*> s_clipPrototypes;
};
