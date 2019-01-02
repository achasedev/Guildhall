/************************************************************************/
/* File: VoxelAnimator.cpp
/* Author: Andrew Chase
/* Date: October 2nd 2018
/* Description: Implementation of the VoxelAnimator class
/************************************************************************/
#include "Game/Framework/Game.hpp"
#include "Game/Animation/VoxelAnimationSet.hpp"
#include "Game/Animation/VoxelAnimator.hpp"
#include "Game/Animation/VoxelAnimation.hpp"
#include "Engine/Core/Time/Stopwatch.hpp"


//-----------------------------------------------------------------------------------------------
// Constructor - takes the set of names for animations to use and a default sprite if a given alias
// doesn't exist
//
VoxelAnimator::VoxelAnimator(const VoxelAnimationSet* animSet, const VoxelSprite* defaultSprite)
	: m_animationSet(animSet)
	, m_defaultSprite(defaultSprite)
{
	m_stopwatch = new Stopwatch(Game::GetGameClock());
}


//-----------------------------------------------------------------------------------------------
// Sets the current animation to play to be one given by the alias, overriding the playmode with the
// given override
//
void VoxelAnimator::Play(const std::string& animationAlias, ePlayMode modeOverride /*= PLAYMODE_DEFAULT*/)
{
	std::string nameToUse = animationAlias;

	// If we have a set, get a translation
	if (m_animationSet != nullptr)
	{
		std::string translation;
		bool foundTranslation = m_animationSet->TranslateAlias(animationAlias, translation);

		if (foundTranslation)
		{
			nameToUse = translation;
		}
	}

	// Only reset the stopwatch if it is a different animation
	const VoxelAnimation* animation = VoxelAnimation::GetAnimationClip(nameToUse);
	if (animation != m_currentAnimation)
	{
		m_currentAnimation = animation;
		m_stopwatch->Reset();
	}

	m_playmode = modeOverride;
}


//-----------------------------------------------------------------------------------------------
// Returns the current sprite to render based on the time into the current animation
//
const VoxelSprite* VoxelAnimator::GetCurrentSprite() const
{
	if (m_currentAnimation != nullptr)
	{
		return m_currentAnimation->Evaluate(m_stopwatch->GetElapsedTime(), m_playmode);
	}

	return m_defaultSprite;
}


//-----------------------------------------------------------------------------------------------
// Returns the default sprite of the animator
//
const VoxelSprite* VoxelAnimator::GetDefaultSprite() const
{
	return m_defaultSprite;
}


//-----------------------------------------------------------------------------------------------
// Returns true if the elapsed time is passed the duration of the current animation
//
bool VoxelAnimator::IsCurrentAnimationFinished() const
{
	float currAnimDuration = m_currentAnimation->GetTotalDuration();
	return (m_stopwatch->GetElapsedTime() >= currAnimDuration);
}
