/************************************************************************/
/* File: BehaviorComponent_PursueDirect.hpp
/* Author: Andrew Chase
/* Date: October 15th 2018
/* Description: Behavior where the entity moves to the closest player directly
/************************************************************************/
#pragma once
#include "Game/Entity/Components/BehaviorComponent.hpp"

class BehaviorComponent_PursueDirect : public BehaviorComponent
{
public:
	//-----Public Methods-----
	
	BehaviorComponent_PursueDirect() {}
	~BehaviorComponent_PursueDirect() {}

	virtual void Update() override;
	virtual BehaviorComponent* Clone() const override;


public:
	//-----Public Data-----
	

private:
	//-----Private Methods-----
	
	
private:
	//-----Private Data-----
	
	
};
