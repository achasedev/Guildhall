/************************************************************************/
/* File: BehaviorComponent.hpp
/* Author: Andrew Chase
/* Date: October 9th, 2018
/* Description: Class to hold any AI state and data of an entity
/************************************************************************/
#pragma once

class MovingEntity;
class HeatMap;

class BehaviorComponent
{
public:
	//-----Public Methods-----
	
	BehaviorComponent();
	virtual ~BehaviorComponent();

	virtual void Initialize(MovingEntity* owningEntity);
	virtual void Update();
	virtual BehaviorComponent* Clone() const = 0;

	
protected:
	//-----Protected Data-----
	
	MovingEntity*	m_owningEntity = nullptr;

};
