/************************************************************************/
/* File: MapGenStep_SpawnItem.hpp
/* Author: Andrew Chase
/* Date: November 27th, 2017
/* Bugs: None
/* Description: (sub)class to represent a map manipulation method
/*				Spawns an item of a given type on the map
/************************************************************************/
#pragma once
#include "Game/MapGenStep.hpp"

typedef tinyxml2::XMLElement XMLElement;
class ItemDefinition;
class TileDefinition;


class MapGenStep_SpawnItem : public MapGenStep
{

public:
	//-----Public Methods-----

	MapGenStep_SpawnItem( const XMLElement& genStepXmlElement );
	virtual ~MapGenStep_SpawnItem() override;

	virtual void Run(Map& mapToAugment) override;


private:
	//-----Private Methods-----

	void SpawnItem(Map& mapToAugment);

private:
	//-----Private Data-----

	std::string m_itemName;						// Name of the Actor, not the ActorDefinition
	const ItemDefinition* m_itemDefinition;		// Actor to spawn
	const TileDefinition* m_tileToSpawnOn;		// Tile type to spawn on 

};
