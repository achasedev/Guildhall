/************************************************************************/
/* File: Animator.hpp
/* Author: Andrew Chase
/* Date: October 1st, 2018
/* Description: Class to represent the animation state of an entity
/************************************************************************/
#pragma once
#include "Game/Animation/VoxelAnimation.hpp"

class Stopwatch;
class VoxelTexture;
class VoxelAnimationSet;
class VoxelAnimation;

class VoxelAnimator
{
public:
	//-----Public Methods-----
	
	VoxelAnimator(const VoxelAnimationSet* animSet, const VoxelSprite* defaultSprite);
	
	void				Play(const std::string& animationAlias, ePlayMode modeOverride = PLAYMODE_DEFAULT);

	const VoxelSprite*	GetCurrentSprite() const;
	bool				IsCurrentAnimationFinished() const;
	

private:
	//-----Private Data-----
	
	Stopwatch*					m_stopwatch			= nullptr;
	const VoxelAnimationSet*	m_animationSet		= nullptr;
	ePlayMode					m_playmode			= PLAYMODE_DEFAULT;
	const VoxelSprite*			m_defaultSprite		= nullptr;
	const VoxelAnimation*		m_currentAnimation	= nullptr;

};
