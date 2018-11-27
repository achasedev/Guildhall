/************************************************************************/
/* File: BehaviorComponent_PursueJump.hpp
/* Author: Andrew Chase
/* Date: October 15th 2018
/* Description: Behavior that moves entity directly to player, jumping over obstacles
/************************************************************************/
#pragma once
#include "Game/Entity/Components/BehaviorComponent.hpp"

class BehaviorComponent_PursueJump : public BehaviorComponent
{
public:
	//-----Public Methods-----
	
	BehaviorComponent_PursueJump();

	virtual void				Update() override;
	virtual BehaviorComponent*	Clone() const override;
	
};
