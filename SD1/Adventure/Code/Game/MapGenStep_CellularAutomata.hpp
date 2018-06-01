/************************************************************************/
/* File: MapGenStep_CellularAutomata.hpp
/* Author: Andrew Chase
/* Date: November 5th, 2017
/* Bugs: None
/* Description: (sub)class to represent a map manipulation method
/*				Changes a tile to a given type based on its neighbors
/************************************************************************/
#pragma once
#include "Game/MapGenStep.hpp"
#include "Engine/Math/IntVector2.hpp"
#include "ThirdParty/tinyxml2/tinyxml2.h"

class TileDefinition;

class MapGenStep_CellularAutomata : public MapGenStep
{
public:
	//-----Public Methods-----

	MapGenStep_CellularAutomata( const tinyxml2::XMLElement& genStepXmlElement );
	virtual ~MapGenStep_CellularAutomata() override;

	virtual void Run(Map& mapToAugment) override;


private:
	//-----Private Methods-----

	void	ApplyCellularAutomata(Map& mapToAugment);
	int		CountNeighborsOfNeighborType(const IntVector2& currCoords, Map& mapToAugment) const;


private:
	//-----Private Data-----

	const TileDefinition*		m_ifType			= nullptr;
	const TileDefinition*		m_toType			= nullptr;
	const TileDefinition*		m_ifNeighborType	= nullptr;

	IntRange					m_ifNeighborCount	= IntRange(1,8);
	float						m_chanceToMutate	= 1.f;

};
