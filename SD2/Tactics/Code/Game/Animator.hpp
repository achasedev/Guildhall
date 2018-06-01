/************************************************************************/
/* File: Animator.hpp
/* Author: Andrew Chase
/* Date: March 14th, 2017
/* Description: Class to represent an animator player per-entity
/************************************************************************/
#pragma once
#include "Game/IsoAnimation.hpp"

class Stopwatch;
class AnimationSet;

class Animator
{
public:
	//-----Public Methods-----

	Animator(AnimationSet* animationSet, IsoSprite* defaultSprite);

	// Mutators
	void Play(const std::string& animName, ePlayMode modeOverride = PLAYMODE_DEFAULT);

	// Accessor/Producers
	IsoSprite*	GetCurrentSprite() const;
	bool		IsCurrentAnimationFinished() const;


private:
	//-----Private Data-----

	// State
	Stopwatch*		m_stopwatch;
	ePlayMode		m_playMode;

	// Resource Data
	AnimationSet*	m_animationSet;
	IsoAnimation*	m_currentAnimation;

	IsoSprite*		m_defaultSprite;

};
