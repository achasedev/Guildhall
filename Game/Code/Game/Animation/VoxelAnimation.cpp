#include "Game/Animation/VoxelAnimation.hpp"

std::map<std::string, const VoxelAnimation*> VoxelAnimation::s_clipPrototypes;

VoxelAnimation::VoxelAnimation(const XMLElement& animElement)
{
	fds
}

std::string VoxelAnimation::GetName() const
{
	return m_name;
}

VoxelSprite* VoxelAnimation::Evaluate(float timeIntoAnimation, ePlayMode modeOverride) const
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

const VoxelAnimation* VoxelAnimation::GetAnimationClip(const std::string& name)
{
	bool clipExists = s_clipPrototypes.find(name) != s_clipPrototypes.end();

	if (clipExists)
	{
		return s_clipPrototypes.at(name);
	}

	return nullptr;
}

