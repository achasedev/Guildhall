/************************************************************************/
/* File: Animator.hpp
/* Author: Andrew Chase
/* Date: October 1st, 2018
/* Description: Class to represent the animation state of an entity
/************************************************************************/
#pragma once

class Stopwatch;
class Texture3D;
class AnimationSet;

class Animator
{
public:
	//-----Public Methods-----
	
	
public:
	//-----Public Data-----
	

private:
	//-----Private Methods-----
	
	
private:
	//-----Private Data-----
	
	Stopwatch*				m_stopwatch = nullptr;
	const AnimationSet*		m_animationSet = nullptr;

	Texture3D*				m_defaultFrame = nullptr;
	
};
