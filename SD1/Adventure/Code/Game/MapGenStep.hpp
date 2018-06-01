/************************************************************************/
/* File: MapGenStep.hpp
/* Author: Andrew Chase
/* Date: November 2nd, 2017
/* Bugs: None
/* Description: Abstract class to represent a map generation method
/************************************************************************/
#pragma once
#include <string>
#include "ThirdParty/tinyxml2/tinyxml2.h"
#include "Engine/Math/IntRange.hpp"

class Map;
typedef tinyxml2::XMLElement XMLElement;

class MapGenStep
{
public:
	//-----Public Methods-----

	MapGenStep(const XMLElement& mapGenerationElement);
	virtual ~MapGenStep();
	virtual void Run(Map& mapToAugment) = 0;	// Pure virtual function, only defined in subclasses

	static MapGenStep* CreateMapGenStep(const XMLElement& genStepElement);


protected:
	//-----Protected Data-----

	std::string m_name = "";
	float m_chanceToRun = 1.0f;
	IntRange m_iterationsRange = IntRange(1,1);
};