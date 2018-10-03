/************************************************************************/
/* File: AIEntity.hpp
/* Author: Andrew Chase
/* Date: October 2nd, 2018
/* Description: Class to represent an AI controlled entity in the game
/************************************************************************/
#pragma once
#include "Game/Entity/DynamicEntity.hpp"

class AIEntity : public DynamicEntity
{
public:
	//-----Public Methods-----
	
	AIEntity(const EntityDefinition* definition);

	
private:
	//-----Private Data-----
	
	// AI controller stuff here
	
};
