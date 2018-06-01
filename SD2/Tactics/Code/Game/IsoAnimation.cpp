/************************************************************************/
/* File: IsoAnimation.cpp
/* Author: Andrew Chase
/* Date: March 14th, 2017
/* Description: Implementation of the IsoAnimation class
/************************************************************************/
#include "Game/IsoSprite.hpp"
#include "Game/GameCommon.hpp"
#include "Game/IsoAnimation.hpp"
#include "Engine/Core/XmlUtilities.hpp"

// All the IsoAnimations in the system are stored here
std::map<std::string, IsoAnimation*> IsoAnimation::s_animations;


//-----------------------------------------------------------------------------------------------
// Converts the string representation of a play mode to the appropriate enumeration
//
ePlayMode IsoAnimation::ConvertStringToPlayMode(const std::string& modeText)
{
	if		(modeText == "loop")	{ return PLAYMODE_LOOP; }
	else if (modeText == "clamp")	{ return PLAYMODE_CLAMP; }
	else { ERROR_AND_DIE(Stringf("Error: IsoAnimation::ConvertStringToPlayMode() received bad mode text: \"%s\"", modeText.c_str())); }
}


//-----------------------------------------------------------------------------------------------
// Adds the given animation to the registry, checking for duplicates
//
void IsoAnimation::AddAnimation(IsoAnimation* animation)
{
	std::string animationName = animation->GetName();
	bool alreadyExists = (s_animations.find(animationName) != s_animations.end());
	GUARANTEE_RECOVERABLE(!alreadyExists, Stringf("Warning: IsoAnimation::AddDefinition() attempted to add duplicate animation \"%s\"", animationName.c_str()));

	s_animations[animationName] = animation;
}


//-----------------------------------------------------------------------------------------------
// Constructs an IsoAnimation from an XMLElement, does not add itself to the registry
//
IsoAnimation::IsoAnimation(const std::string& name, const XMLElement& animElement)
	: m_name(name)
{
	// Parse the playmode
	std::string modeText	= ParseXmlAttribute(animElement, "playmode", "clamp");
	m_defaultPlayMode		= ConvertStringToPlayMode(modeText);

	// Parse the frames
	const XMLElement* frameElement = animElement.FirstChildElement();

	while (frameElement != nullptr)
	{
		// Parse the isosprite
		std::string isoSpriteName = ParseXmlAttribute(*frameElement, "isosprite");
		IsoSprite* isoSprite = IsoSprite::GetIsoSprite(isoSpriteName);
		GUARANTEE_OR_DIE(isoSprite != nullptr, Stringf("Error: IsoAnimation constructor - IsoSprite \"%s\" doesn't exist.", isoSpriteName.c_str()));

		// Parse the duration
		float frameDuration = ParseXmlAttribute(*frameElement, "duration", 1.0f);

		// Add the frame to this animation
		m_frames.push_back(AnimFrame(isoSprite, frameDuration));

		// Move on to the next frame element
		frameElement = frameElement->NextSiblingElement();
	}
}


//-----------------------------------------------------------------------------------------------
// Returns the name of the animation
//
std::string IsoAnimation::GetName() const
{
	return m_name;
}


//-----------------------------------------------------------------------------------------------
// Returns the default PlayMode of this animation
//
ePlayMode IsoAnimation::GetDefaultPlayMode() const
{
	return m_defaultPlayMode;
}


//-----------------------------------------------------------------------------------------------
// Finds the frame of this animation at the given time, under the given play mode
//
IsoSprite* IsoAnimation::Evaluate(float timeIntoAnimation, ePlayMode modeOverride) const
{
	// Use the Animation's play mode, or the one from the animator?
	ePlayMode playMode = (modeOverride == PLAYMODE_DEFAULT ? m_defaultPlayMode : modeOverride);

	float totalDuration = GetTotalDuration();

	// If we're over the end of the animation and clamped, return the last frame
	if (playMode == PLAYMODE_CLAMP && timeIntoAnimation >= totalDuration)
	{
		return m_frames.back().m_sprite;
	}

	// Otherwise perform "mod-like" arithmetic to find the current frame
	while (timeIntoAnimation >= totalDuration)
	{
		timeIntoAnimation -= totalDuration;
	}
	
	int numFrames = (int) m_frames.size();
	float timeSum = 0.f;
	int returnIndex = 0;

	for (int frameIndex = 0; frameIndex < numFrames; ++frameIndex)
	{
		timeSum += m_frames[frameIndex].m_duration;

		if (timeIntoAnimation < timeSum)
		{
			returnIndex = frameIndex;
			break;
		}
	}

	return m_frames[returnIndex].m_sprite;
}


//-----------------------------------------------------------------------------------------------
// Calculates and returns the total sum of all frame durations
//
float IsoAnimation::GetTotalDuration() const
{
	int numFrames = (int) m_frames.size();
	float totalDuration = 0.f;

	for (int frameIndex = 0; frameIndex < numFrames; ++frameIndex)
	{
		totalDuration += m_frames[frameIndex].m_duration;
	}

	return totalDuration;
}


//-----------------------------------------------------------------------------------------------
// Loads the XMLDocument given by filePath and constructs the IsoAnimations
// Assumes all specified sprites already exist!
//
void IsoAnimation::LoadIsoAnimations(const std::string& filePath)
{
	// Load the document
	XMLDocument document;
	XMLError error = document.LoadFile(filePath.c_str());
	GUARANTEE_OR_DIE(error == tinyxml2::XML_SUCCESS, Stringf("Error: IsoAnimation::LoadIsoAnimations() couldn't load file \"%s\"", filePath.c_str()));

	// I keep a root around just because I like having a single root element
	XMLElement* rootElement = document.RootElement();
	XMLElement* animElement = rootElement->FirstChildElement();

	while (animElement != nullptr)
	{
		// Construct an IsoAnimation and add it to the registry
		std::string animName = ParseXmlAttribute(*animElement, "name");
		IsoAnimation* animation = new IsoAnimation(animName, *animElement);
		AddAnimation(animation);

		animElement = animElement->NextSiblingElement();
	}
}


//-----------------------------------------------------------------------------------------------
// Returns the IsoAnimation with the given name, or nullptr if it doesn't exist
//
IsoAnimation* IsoAnimation::GetAnimation(const std::string& name)
{
	bool animationExists = (s_animations.find(name) != s_animations.end());

	if (animationExists)
	{
		return s_animations[name];
	}
	else
	{
		return nullptr;
	}
}
