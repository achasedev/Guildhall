/************************************************************************/
/* File: VoxelAnimation.cpp
/* Author: Andrew Chase
/* Date: October 2nd, 2018
/* Description: Implementation of the VoxelAnimation class
/************************************************************************/
#include "Game/Animation/VoxelAnimation.hpp"
#include "Engine/Core/Utility/StringUtils.hpp"
#include "Engine/Core/Utility/ErrorWarningAssert.hpp"

// Global list for all animations in the game
std::map<std::string, const VoxelAnimation*> VoxelAnimation::s_voxelAnimations;


//-----------------------------------------------------------------------------------------------
// Returns the enum representation of the playmode given the string representation
//
ePlayMode ConvertStringToPlaymode(const std::string& modeText)
{
	if		(modeText == "loop")	{ return PLAYMODE_LOOP; }
	else if (modeText == "clamp")	{ return PLAYMODE_CLAMP; }
	else if (modeText == "default") { return PLAYMODE_DEFAULT; }
	else
	{
		ERROR_AND_DIE(Stringf("Error:: ConvertStringToPlaymode() received bad mode string: %s", modeText.c_str()));
	}
}


//-----------------------------------------------------------------------------------------------
// Constructs an animation given the XML element
//
VoxelAnimation::VoxelAnimation(const XMLElement& animElement)
{
	// Get the name
	m_name = ParseXmlAttribute(animElement, "name");

	if (m_name.size() == 0)
	{
		ERROR_AND_DIE("Error: VoxelAnimation::VoxelAnimation() received animation element with no name");
		return;
	}

	// Get the playmode
	std::string modeString = ParseXmlAttribute(animElement, "playmode", "default");
	m_playMode = ConvertStringToPlaymode(modeString);

	// Parse the frames
	const XMLElement* frameElement = animElement.FirstChildElement();

	while (frameElement != nullptr)
	{
		// Parse the Voxel sprite
		std::string voxelSpriteName = ParseXmlAttribute(*frameElement, "voxelsprite");
		const VoxelSprite* voxelSprite = VoxelSprite::GetVoxelSprite(voxelSpriteName);

		GUARANTEE_OR_DIE(voxelSprite != nullptr, Stringf("Error: VoxelAnimation::VoxelAnimation() contained sprite that doesn't exist:\"%s\"", voxelSpriteName.c_str()));

		// Parse the duration
		float frameDuration = ParseXmlAttribute(*frameElement, "duration", 1.0f);

		// Add the frame to this animation
		m_frames.push_back(VoxelAnimFrame(voxelSprite, frameDuration));

		// Move on to the next frame element
		frameElement = frameElement->NextSiblingElement();
	}
}


//-----------------------------------------------------------------------------------------------
// Returns the name of the animation
//
std::string VoxelAnimation::GetName() const
{
	return m_name;
}


//-----------------------------------------------------------------------------------------------
// Returns the sprite for the given time into animation and mode override
//
const VoxelSprite* VoxelAnimation::Evaluate(float timeIntoAnimation, ePlayMode modeOverride) const
{
	// Use the Animation's play mode, or the one from the animator?
	ePlayMode playMode = (modeOverride == PLAYMODE_DEFAULT ? m_playMode : modeOverride);

	float totalDuration = GetTotalDuration();

	// If we're at the end and need to clamp, return the end
	if (playMode == PLAYMODE_CLAMP && timeIntoAnimation >= totalDuration)
	{
		return m_frames.back().sprite;
	}

	// Otherwise use mod arithmetic to return a looped animation
	while (timeIntoAnimation >= totalDuration)
	{
		timeIntoAnimation -= totalDuration;
	}

	int numFrames = (int)m_frames.size();

	float timeSum = 0.f;
	int returnIndex = 0;

	for (int frameIndex = 0; frameIndex < numFrames; ++frameIndex)
	{
		timeSum += m_frames[frameIndex].duration;

		if (timeIntoAnimation < timeSum)
		{
			returnIndex = frameIndex;
			break;
		}
	}

	return m_frames[returnIndex].sprite;
}


//-----------------------------------------------------------------------------------------------
// Returns the total duration of the animation (sum of all frame durations)
//
float VoxelAnimation::GetTotalDuration() const
{
	int numFrames = (int)m_frames.size();

	float totalDuration = 0.f;
	for (int frameIndex = 0; frameIndex < numFrames; ++frameIndex)
	{
		totalDuration += m_frames[frameIndex].duration;
	}

	return totalDuration;
}


//-----------------------------------------------------------------------------------------------
// Loads the animations from the XML file given by filename
//
void VoxelAnimation::LoadVoxelAnimations(const std::string& filename)
{
	XMLDocument document;
	XMLError error = document.LoadFile(filename.c_str());

	if (error != tinyxml2::XML_SUCCESS)
	{
		ERROR_AND_DIE(Stringf("Error: VoxelAnimation::LoadVoxelAnimations() couldn't open XML file %s", filename.c_str()));
		return;
	}

	const XMLElement* rootElement = document.RootElement();

	if (rootElement == nullptr)
	{
		ERROR_AND_DIE(Stringf("Error: VoxelAnimation::LoadVoxelAnimations() loaded file with no root element: %s", filename.c_str()));
		return;
	}

	const XMLElement* animElement = rootElement->FirstChildElement();

	while (animElement != nullptr)
	{
		VoxelAnimation* animation = new VoxelAnimation(*animElement);
		s_voxelAnimations[animation->GetName()] = animation;
		
		animElement = animElement->NextSiblingElement();
	}
}


//-----------------------------------------------------------------------------------------------
// Returns the animation from the global list given by name, nullptr if it doesn't exist
//
const VoxelAnimation* VoxelAnimation::GetAnimationClip(const std::string& name)
{
	bool clipExists = s_voxelAnimations.find(name) != s_voxelAnimations.end();

	if (clipExists)
	{
		return s_voxelAnimations.at(name);
	}

	return nullptr;
}
