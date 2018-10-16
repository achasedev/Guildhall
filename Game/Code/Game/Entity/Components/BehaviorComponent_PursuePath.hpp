/************************************************************************/
/* File: BehaviorComponent_Wander.hpp
/* Author: Andrew Chase
/* Date: October 9th, 2018
/* Description: Behavior of an entity pathfinding to the nearest player
/************************************************************************/
#pragma once
#include "Game/Entity/Components/BehaviorComponent.hpp"

class BehaviorComponent_PursuePath : public BehaviorComponent
{
public:
	//-----Public Methods-----
	
	BehaviorComponent_PursuePath();
	~BehaviorComponent_PursuePath();
	
	virtual void				Update() override;
	virtual BehaviorComponent*	Clone() const override;

};
