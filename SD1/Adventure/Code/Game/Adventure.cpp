/************************************************************************/
/* File: Adventure.cpp
/* Author: Andrew Chase
/* Date: November 19th, 2017
/* Bugs: None
/* Description: Implementation of the Adventure class
/************************************************************************/
#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
#include "Game/Adventure.hpp"
#include "Game/Map.hpp"
#include "Game/Player.hpp"
#include "Game/Camera.hpp"
#include "Game/Item.hpp"
#include "Game/ActorDefinition.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"


//-----------------------------------------------------------------------------------------------
// Constructs an adventure from an adventure definition by generating the maps and spawning entities
//
Adventure::Adventure(const AdventureDefinition* adventureDefinition)
	: m_adventureDefinition(adventureDefinition)
{
	// Construct all maps before spawning entities
	GenerateMaps(adventureDefinition);

	// Spawn entities
	for (int i = 0; i < static_cast<int>(adventureDefinition->m_mapsToGenerate.size()); i++)
	{
		MapToGenerate* currMapGenData = adventureDefinition->m_mapsToGenerate[i];
		Map* currMap = m_maps[currMapGenData->m_name];

		SpawnActorsOnMap(currMap, currMapGenData);
		SpawnPortalsOnMap(currMap, currMapGenData);
	}

	// Set the active map to the start map
	m_activeMap = m_maps.at(adventureDefinition->m_startMapName);

	// Create the player and add them to the start map
	const ActorDefinition* playerDefinition = ActorDefinition::GetDefinitionByName("Player");
	g_thePlayer = new Player(Vector2(3.5f, 3.5f), 0.f, playerDefinition, m_activeMap);

	// Put the player on the start tile type
	Vector2 playerStartPos = m_activeMap->GetRandomTileOfType(adventureDefinition->m_startTileType)->GetCenterPosition();
	g_thePlayer->SetPosition(playerStartPos);
	m_activeMap->AddActorToMap(g_thePlayer);

	// Create the camera and add them to the map
	g_theCamera = new Camera();
}


//-----------------------------------------------------------------------------------------------
// Destructor - delete the map of game maps AND deletes the player
//
Adventure::~Adventure()
{
	// Free the list of maps
	std::map<std::string, Map*>::iterator itr = m_maps.begin();

	for (itr; itr != m_maps.end(); itr++)
	{
		delete itr->second;
	}

	// Delete the player character
	delete g_thePlayer;
	g_thePlayer = nullptr;

	// Delete the camera
	delete g_theCamera;
	g_theCamera = nullptr;
}


//-----------------------------------------------------------------------------------------------
// Updates the active map of this adventure, and checks for map transitions
//
void Adventure::Update(float deltaTime)
{
	// Check if we need to switch maps
	if (m_transitionToMap != nullptr)
	{
		m_activeMap = m_transitionToMap;
		m_transitionToMap = nullptr;

		// Also disable all portals on this map when first entering
		m_activeMap->DisableAllPortals();

		// Also update the song based on the new map
	}

	g_theCamera->Update(deltaTime);
	m_activeMap->Update(deltaTime);
}


//-----------------------------------------------------------------------------------------------
// Draws the active map to screen
//
void Adventure::Render() const
{
	g_theCamera->SetPlayerView(m_activeMap->GetDimensions());
	m_activeMap->Render();
}


//-----------------------------------------------------------------------------------------------
// Sets the transition map member to the passed map, switching to that map at the start of the next frame
//
void Adventure::TransitionToMap(Map* mapToMoveTo)
{
	m_transitionToMap = mapToMoveTo;
}


//-----------------------------------------------------------------------------------------------
// Checks if the entity name passed was the name of the boss to defeat to win the game, and if so
// tell the game to enter the victory state
//
void Adventure::CheckForVictory(const std::string& nameOfActorThatDied)
{
	if (m_adventureDefinition->m_nameOfEnemyToDefeat.compare(nameOfActorThatDied) == 0)
	{
		g_theGame->StartTransitionToState(GAME_STATE_VICTORY, true);
	}
}


//-----------------------------------------------------------------------------------------------
// Generates the maps for this adventure given the adventure definition
// Constructs the tiles and environment, but does not spawn entities
//
void Adventure::GenerateMaps(const AdventureDefinition* adventureDefinition)
{
	// Generate the maps and add them to m_maps
	for (int i = 0; i < static_cast<int>(adventureDefinition->m_mapsToGenerate.size()); i++)
	{
		MapToGenerate* currMapGenData = adventureDefinition->m_mapsToGenerate[i];
		std::string currMapName = currMapGenData->m_name;
		Map* newMap = new Map(currMapName, currMapGenData->m_mapDef, this);

		// Safety check against duplicate maps names
		bool mapAlreadyExists = m_maps.find(currMapName) != m_maps.end();
		GUARANTEE_OR_DIE(!mapAlreadyExists, Stringf("Error: Adventure::Adventure() - Duplicate map of name \"%s\"", currMapName.c_str()));

		m_maps[currMapName] = newMap;
	}
}


//-----------------------------------------------------------------------------------------------
// Spawns the actors on all maps in this adventure (maps should be generated first)
//
void Adventure::SpawnActorsOnMap(Map* mapToSpawnOn, MapToGenerate* generateData)
{
	std::vector<ActorToSpawn*>& actorsToSpawn = generateData->m_actorsToSpawn;

	for (int actorIndex = 0; actorIndex < static_cast<int>(actorsToSpawn.size()); actorIndex++)
	{
		ActorToSpawn* currActorToSpawn = actorsToSpawn[actorIndex];
		Vector2 spawnPosition = mapToSpawnOn->GetRandomTileOfType(currActorToSpawn->m_tileDefToSpawnOn)->GetCenterPosition();
		mapToSpawnOn->SpawnActor(spawnPosition, 0.f, currActorToSpawn->m_actorDef, currActorToSpawn->m_actorName);
	}
}


//-----------------------------------------------------------------------------------------------
// Spawns the portals on all maps in this adventure (maps should be generated first)
//
void Adventure::SpawnPortalsOnMap(Map* mapToSpawnOn, MapToGenerate* generateData)
{
	std::vector<PortalToSpawn*>& portalsToSpawn = generateData->m_portalsToSpawn;

	for (int portalIndex = 0; portalIndex < static_cast<int>(portalsToSpawn.size()); portalIndex++)
	{
		PortalToSpawn* currPortalToSpawn = portalsToSpawn[portalIndex];
		std::string destinationMapName = currPortalToSpawn->m_destinationMapName;

		// Safety check - ensure destination map exists
		bool destinationMapExists = m_maps.find(destinationMapName) != m_maps.end();
		GUARANTEE_OR_DIE(destinationMapExists, Stringf("Error: Adventure::SpawnPortalsOnMap() - Destination map of name \"%s\" doesn't exist", destinationMapName.c_str()));
		Map* destinationMap = m_maps[destinationMapName];

		// Get the portal end positions
		Vector2 spawnPosition = mapToSpawnOn->GetRandomTileOfType(currPortalToSpawn->m_tileDefToSpawnOn)->GetCenterPosition();
		Vector2 destinationPosition = destinationMap->GetRandomTileOfType(currPortalToSpawn->m_tileDefToTeleportTo)->GetCenterPosition();

		mapToSpawnOn->SpawnPortal(spawnPosition, 0.f, currPortalToSpawn->m_portalDef, "", destinationMap, destinationPosition);

		// If we have a reciprocal portal, spawn it
		if (currPortalToSpawn->m_reciprocalPortalType != nullptr)
		{
			destinationMap->SpawnPortal(destinationPosition, 0.f, currPortalToSpawn->m_reciprocalPortalType, "", mapToSpawnOn, spawnPosition);
		}
	}
}
