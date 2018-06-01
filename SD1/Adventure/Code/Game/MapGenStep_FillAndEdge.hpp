/************************************************************************/
/* File: MapGenStep_FillAndEdge.hpp
/* Author: Andrew Chase
/* Date: November 2nd, 2017
/* Bugs: None
/* Description: (sub)class to represent a map manipulation method
/*				Fills the map with a certain tile type, and then lines the
/*				map edge with a different type
/************************************************************************/
#pragma once
#include "Game/MapGenStep.hpp"
#include "ThirdParty/tinyxml2/tinyxml2.h"

class TileDefinition;

class MapGenStep_FillAndEdge : public MapGenStep
{
public:
	//-----Public Methods-----

	MapGenStep_FillAndEdge( const tinyxml2::XMLElement& genStepXmlElement );
	virtual ~MapGenStep_FillAndEdge() override;

	virtual void Run(Map& mapToAugment) override;

	
private:
	//-----Private Methods-----
	void ApplyFillAndEdge(Map& mapToAugment);

private:
	//-----Private Data-----

	const TileDefinition*		m_fillTileDef = nullptr;
	const TileDefinition*		m_edgeTileDef = nullptr;
	int							m_edgeThickness = 1;

};