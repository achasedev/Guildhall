/************************************************************************/
/* File: BehaviorComponent.hpp
/* Author: Andrew Chase
/* Date: October 9th, 2018
/* Description: Class to hold any AI state and data of an entity
/************************************************************************/
#pragma once

class AnimatedEntity;

class BehaviorComponent
{
public:
	//-----Public Methods-----
	
	BehaviorComponent();
	virtual ~BehaviorComponent();

	virtual void				Initialize(AnimatedEntity* owningEntity);
	virtual void				Update();
	virtual BehaviorComponent*	Clone() const = 0;

	
protected:
	//-----Protected Data-----
	
	AnimatedEntity*	m_owningEntity = nullptr;

};
