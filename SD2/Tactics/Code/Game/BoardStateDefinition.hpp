/************************************************************************/
/* File: BoardStateDefinition.hpp
/* Author: Andrew Chase
/* Date: March 23rd, 2018
/* Description: Class for holding BoardState XML data
/************************************************************************/
#pragma once
#include <map>
#include <string>
#include "Engine/Math/IntVector2.hpp"
#include "Engine/Core/XmlUtilities.hpp"

class ActorDefinition;

// Struct to represent info needed to spawn a single actor
struct ActorSpawnInfo
{
	ActorSpawnInfo(const std::string& name, ActorDefinition* definition, const IntVector2& spawnPosition, int teamIndex)
		: m_name(name), m_definition(definition), m_spawnPosition(spawnPosition), m_teamIndex(teamIndex) {}

	std::string m_name;
	ActorDefinition* m_definition;
	IntVector2 m_spawnPosition;
	int m_teamIndex;
};


class BoardStateDefinition
{
public:
	//-----Public Methods-----

	BoardStateDefinition(const XMLElement& definitionElement);

	std::string GetName() const;
	std::string GetMapFilePath() const;
	const std::vector<ActorSpawnInfo>& GetActorSpawnInfo() const;

	// Statics
	static void LoadDefinitions(const std::string& filePath);
	static void AddDefinition(BoardStateDefinition* definition);
	static BoardStateDefinition* GetDefinition(const std::string& definitionName);

	static const std::map<std::string, BoardStateDefinition*>& GetAllDefinitions();


private:
	//-----Private Data-----

	std::string m_name;
	std::string m_mapFilePath;

	std::vector<ActorSpawnInfo> m_actorsToSpawn;

	static std::map<std::string, BoardStateDefinition*> s_definitions;

};
