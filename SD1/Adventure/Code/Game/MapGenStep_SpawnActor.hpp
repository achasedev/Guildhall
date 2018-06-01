/************************************************************************/
/* File: MapGenStep_SpawnActor.hpp
/* Author: Andrew Chase
/* Date: November 27th, 2017
/* Bugs: None
/* Description: (sub)class to represent a map manipulation method
/*				Spawns an actor of a given type on the map
/************************************************************************/
#pragma once
#include "Game/MapGenStep.hpp"

typedef tinyxml2::XMLElement XMLElement;
class ActorDefinition;
class TileDefinition;


class MapGenStep_SpawnActor : public MapGenStep
{

public:
	//-----Public Methods-----

	MapGenStep_SpawnActor( const XMLElement& genStepXmlElement );
	virtual ~MapGenStep_SpawnActor() override;

	virtual void Run(Map& mapToAugment) override;


private:
	//-----Private Methods-----

	void SpawnActor(Map& mapToAugment);

private:
	//-----Private Data-----

	std::string m_actorName;					// Name of the Actor, not the ActorDefinition
	const ActorDefinition* m_actorDefinition;	// Actor to spawn
	const TileDefinition* m_tileToSpawnOn;		// Tile type to spawn on 

};
