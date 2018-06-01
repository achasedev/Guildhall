/************************************************************************/
/* File: Animator.cpp
/* Author: Andrew Chase
/* Date: March 14th, 2017
/* Description: Implementation of the Animator class
/************************************************************************/
#include "Game/Game.hpp"
#include "Game/Animator.hpp"
#include "Game/AnimationSet.hpp"
#include "Game/IsoAnimation.hpp"
#include "Engine/Core/Stopwatch.hpp"
#include "Engine/Core/StringUtils.hpp"


//-----------------------------------------------------------------------------------------------
// Constructor
//
Animator::Animator(AnimationSet* animationSet, IsoSprite* defaultSprite)
	: m_playMode(PLAYMODE_DEFAULT)
	, m_currentAnimation(nullptr)
	, m_animationSet(animationSet)
	, m_defaultSprite(defaultSprite)
{
	m_stopwatch = new Stopwatch(Game::GetGameClock());
}


//-----------------------------------------------------------------------------------------------
// Begins playing an animation given by the name (or alias) animName
//
void Animator::Play(const std::string& animName, ePlayMode modeOverride /*= PLAYMODE_DEFAULT*/)
{
	std::string nameToUse = animName;
	if (m_animationSet != nullptr) {
		std::string nameTranslation = m_animationSet->TranslateAlias(animName);

		if (!IsStringNullOrEmpty(nameTranslation)) {
			nameToUse = nameTranslation; 
		}
	}

	m_currentAnimation = IsoAnimation::GetAnimation(nameToUse); 
	m_stopwatch->Reset();
	m_playMode = modeOverride;
}


//-----------------------------------------------------------------------------------------------
// Returns the current sprite of the current animation based on play mode and time into sequence
//
IsoSprite* Animator::GetCurrentSprite() const
{
	if (m_currentAnimation != nullptr)
	{
		return m_currentAnimation->Evaluate(m_stopwatch->GetElapsedTime(), m_playMode);
	}
	else 
	{
		return m_defaultSprite;
	}
}


//-----------------------------------------------------------------------------------------------
// Returns true if the time into sequence exceed the current animation's duration
//
bool Animator::IsCurrentAnimationFinished() const
{
	float currAnimDuration = m_currentAnimation->GetTotalDuration();
	return (m_stopwatch->GetElapsedTime() >= currAnimDuration);
}
