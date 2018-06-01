/************************************************************************/
/* File: IsoAnimation.hpp
/* Author: Andrew Chase
/* Date: March 14th, 2017
/* Description: Class represent an animation sequence of Isometric sprites
/************************************************************************/
#pragma once
#include <map>
#include <string>
#include <vector>
#include "Engine/Core/XmlUtilities.hpp"

class IsoSprite;


// Struct to represent a single frame of the animation sequence
struct AnimFrame
{
	AnimFrame(IsoSprite* sprite, float duration)
		: m_sprite(sprite), m_duration(duration)
	{}
	IsoSprite*	m_sprite;
	float		m_duration;
};


// Enumeration to determine how to choose a sprite after the sequence is over
enum ePlayMode
{
	PLAYMODE_DEFAULT, // Used by Animator to use the animation's default mode
	PLAYMODE_CLAMP,
	PLAYMODE_LOOP,
	NUM_PLAYMODES
};

class IsoAnimation
{
public:
	//-----Public Methods-----
	
	// Accessors
	std::string GetName() const;
	ePlayMode	GetDefaultPlayMode() const;
	IsoSprite*	Evaluate(float timeIntoAnimation, ePlayMode modeOverride) const;

	// Producers
	float GetTotalDuration() const;

	// Statics
	static void				LoadIsoAnimations(const std::string& filePath);
	static void				AddAnimation(IsoAnimation* animation);
	static IsoAnimation*	GetAnimation(const std::string& name);


private:
	//-----Private Methods-----

	// Construct from XML, can only be called from LoadDefinitions()
	IsoAnimation(const std::string& name, const XMLElement& animElement);

	// Translates a string representation of a play mode to a play mode
	static ePlayMode ConvertStringToPlayMode(const std::string& modeText);


private:
	//-----Private Data-----

	std::string				m_name;
	ePlayMode				m_defaultPlayMode;
	std::vector<AnimFrame>	m_frames;

	// ALL the IsoAnimations in the system are stored here
	static std::map<std::string, IsoAnimation*> s_animations;
};
