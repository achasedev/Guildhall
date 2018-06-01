/************************************************************************/
/* File: AdventureDefinition.hpp
/* Author: Andrew Chase
/* Date: November 19th, 2017
/* Bugs: None
/* Description: Class to represent Adventure static data from an XML file
/************************************************************************/
#pragma once
#include <string>
#include "Game/MapDefinition.hpp"

class ActorDefinition;
class TileDefinition;
class PortalDefinition;


//--Struct to represent an actor to spawn in a given map--
struct ActorToSpawn
{
	std::string m_actorName;
	const ActorDefinition* m_actorDef;
	const TileDefinition* m_tileDefToSpawnOn;
};

//--Struct to represent a portal to spawn in a given map--
struct PortalToSpawn
{
	const PortalDefinition* m_portalDef;
	const TileDefinition* m_tileDefToSpawnOn;
	const TileDefinition* m_tileDefToTeleportTo;
	const PortalDefinition* m_reciprocalPortalType;	// If this is null, then there is no return portal to this one
	std::string m_destinationMapName;
};

//--Struct to represent a map to generate and spawn entities on for an adventure--
struct MapToGenerate
{
	std::string m_name;
	const MapDefinition* m_mapDef;
	std::vector<ActorToSpawn*> m_actorsToSpawn;
	std::vector<PortalToSpawn*> m_portalsToSpawn;
};


class AdventureDefinition
{
public:

	friend class Adventure;		// Used to allow access to the m_mapsToGenerate vector, for Adventure construction

	//-----Public Methods-----

	AdventureDefinition(const XMLElement& adventureDefElement);

	static void LoadDefinitions();
	static const AdventureDefinition* GetDefinitionByName(const std::string& definitionName);

	// Accessors
	std::string GetName() const;
	
private:
	//-----Private Methods-----

	void ParseStartConditions(const XMLElement& adventureDefElement);
	void ParseVictoryConditions(const XMLElement& adventureDefElement);

	void ParseMapGenerationData(const XMLElement& adventureDefElement);
	void ParseMapActorData(MapToGenerate* mapToGenStruct, const XMLElement& mapElement);
	void ParseMapPortalData(MapToGenerate* mapToGenStruct, const XMLElement& mapElement);


private:
	//-----Private Data-----

	// General Info
	std::string m_name;										// Name of this Adventure
	std::string m_title;									// The display text for this Adventure on the Attract screen

	// Start Conditions
	std::string m_startMapName;								// The name of the first map to be loaded in the adventure
	const TileDefinition* m_startTileType = nullptr;		// The type of tile the player starts on

	// Victory Conditions
	std::string m_nameOfEnemyToDefeat;						// For now, the name of the actor to defeat to win

	// Generation information
	std::vector<MapToGenerate*> m_mapsToGenerate;			// Maps that need to be created for this adventure

	// Dialogue information
	std::vector<std::string> m_dialogueTexts;


	// Static list of definitions loaded from file
	static std::map<std::string, AdventureDefinition*> s_definitions;

};
