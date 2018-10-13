/************************************************************************/
/* File: BehaviorComponent_Wander.hpp
/* Author: Andrew Chase
/* Date: October 9th, 2018
/* Description: Behavior of an entity moving to random locations on an interval
/************************************************************************/
#pragma once
#include "Game/Entity/Components/BehaviorComponent.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Core/Time/Stopwatch.hpp"

class BehaviorComponent_PursuePath : public BehaviorComponent
{
public:
	//-----Public Methods-----
	
	BehaviorComponent_PursuePath();
	~BehaviorComponent_PursuePath();
	

	virtual void Update() override;
	virtual BehaviorComponent* Clone() const override;

};
