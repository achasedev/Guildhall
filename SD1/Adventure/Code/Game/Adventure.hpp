/************************************************************************/
/* File: Adventure.hpp
/* Author: Andrew Chase
/* Date: November 19th, 2017
/* Bugs: None
/* Description: Class to represented a playable adventure
				Consists of a set of maps, entities, and start/victory/
				defeat conditions
/************************************************************************/
#pragma once
#include <map>
#include <string>
#include <queue>
#include "AdventureDefinition.hpp"

class Map;


class Adventure
{
public:
	//-----Public Methods-----

	Adventure(const AdventureDefinition* adventureDefinition);
	~Adventure();

	void Update(float deltaTime);
	void Render() const;

	void TransitionToMap(Map* mapToMoveTo);		// Tells this Adventure to transition to mapToMoveTo at the start of the next frame

	void CheckForVictory(const std::string& nameOfActorThatDied);

private:
	//-----Private Methods-----

	void GenerateMaps(const AdventureDefinition* adventureDefinition);
	void SpawnActorsOnMap(Map* mapToSpawnOn, MapToGenerate* generateData);
	void SpawnPortalsOnMap(Map* mapToSpawnOn, MapToGenerate* generateData);


private:
	//-----Private Data-----

	const AdventureDefinition* m_adventureDefinition;
	std::map<std::string, Map*> m_maps;
	Map* m_activeMap = nullptr;
	Map* m_transitionToMap = nullptr;
};
