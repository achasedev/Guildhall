/************************************************************************/
/* File: Animator.hpp
/* Author: Andrew Chase
/* Date: October 1st, 2018
/* Description: Class to represent the animation state of an entity
/************************************************************************/
#pragma once
#include "Game/Animation/VoxelAnimation.hpp"

class Stopwatch;
class Texture3D;
class AnimationSet;
class VoxelAnimation;

class VoxelAnimator
{
public:
	//-----Public Methods-----
	
	void			Play(const std::string& animationAlias, ePlayMode modeOverride = PLAYMODE_DEFAULT);

	VoxelSprite*	GetCurrentSprite() const;
	bool			IsCurrentAnimationFinished() const;
	

private:
	//-----Private Data-----
	
	Stopwatch*				m_stopwatch = nullptr;
	const AnimationSet*		m_animationSet = nullptr;
	ePlayMode				m_defaultPlayMode = PLAYMODE_DEFAULT;
	Texture3D*				m_defaultFrame = nullptr;
	VoxelAnimation*			m_currentAnimation = nullptr;

};
