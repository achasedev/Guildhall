/************************************************************************/
/* File: MapGenStep_Mutate.hpp
/* Author: Andrew Chase
/* Date: November 18th, 2017
/* Bugs: None
/* Description: (sub)class to represent a map manipulation method
/*				Changes the interior tiles of the map of a given type
/*				to a specified type with a determined probability
/************************************************************************/
#pragma once
#include "Game/MapGenStep.hpp"
#include "ThirdParty/tinyxml2/tinyxml2.h"

class TileDefinition;

class MapGenStep_Mutate : public MapGenStep
{
public:
	//-----Public Methods-----

	MapGenStep_Mutate( const tinyxml2::XMLElement& genStepXmlElement );
	virtual ~MapGenStep_Mutate() override;

	virtual void Run(Map& mapToAugment) override;


private:
	//-----Private Methods-----

	void ApplyMutate(Map& mapToAugment);

private:
	//-----Private Data-----

	const TileDefinition*		m_ifType = nullptr;				// Type of tile to change
	const TileDefinition*		m_toType = nullptr;				// Type to change tiles too
	float						m_mutateChance = 1.0f;			// Default chance for an interior tile to be changed

};