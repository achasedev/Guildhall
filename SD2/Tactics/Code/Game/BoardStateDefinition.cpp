/************************************************************************/
/* File: BoardStateDefinition.cpp
/* Author: Andrew Chase
/* Date: March 23rd, 2018
/* Description: Class to represent spawn data elements for a BoardState
/************************************************************************/
#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"
#include "Game/GameState_Playing.hpp"
#include "Game/BoardStateDefinition.hpp"
#include "Game/ActorDefinition.hpp"
#include "Engine/Core/Command.hpp"
#include "Engine/Core/DevConsole.hpp"

// C command callback for the change boardstate callback
void Command_LoadBoardState(Command& cmd);

// Map for all definitions of this class
std::map<std::string, BoardStateDefinition*> BoardStateDefinition::s_definitions;


//-----------------------------------------------------------------------------------------------
// Parses the data from XML and constructs a definition, adding itself to the static registry
//
BoardStateDefinition::BoardStateDefinition(const XMLElement& definitionElement)
{
	// Parse the name
	m_name = ParseXmlAttribute(definitionElement, "name");

	// Parse the map file path
	m_mapFilePath = ParseXmlAttribute(definitionElement, "map");

	// Parse the actor infos
	const XMLElement* actorElement = definitionElement.FirstChildElement();

	while (actorElement != nullptr)
	{
		std::string actorName = ParseXmlAttribute(*actorElement, "name");
		IntVector2 position = ParseXmlAttribute(*actorElement, "position", IntVector2(0,0));
		int teamIndex = ParseXmlAttribute(*actorElement, "team", 0);
		
		std::string definitionName = ParseXmlAttribute(*actorElement, "definition");
		ActorDefinition* definition = ActorDefinition::GetDefinition(definitionName);

		ActorSpawnInfo info = ActorSpawnInfo(actorName, definition, position, teamIndex);
		m_actorsToSpawn.push_back(info);

		actorElement = actorElement->NextSiblingElement();
	}
}


//-----------------------------------------------------------------------------------------------
// Returns the definition's name
//
std::string BoardStateDefinition::GetName() const
{
	return m_name;
}


//-----------------------------------------------------------------------------------------------
// Returns the file path to the map's height map .png file
//
std::string BoardStateDefinition::GetMapFilePath() const
{
	return m_mapFilePath;
}


//-----------------------------------------------------------------------------------------------
// Returns a reference to all the actor spawn information for this boardstate
//
const std::vector<ActorSpawnInfo>& BoardStateDefinition::GetActorSpawnInfo() const
{
	return m_actorsToSpawn;
}


//-----------------------------------------------------------------------------------------------
// Loads the XML document located at filePath and parses/constructs all definitions in the file
//
void BoardStateDefinition::LoadDefinitions(const std::string& filePath)
{
	// Load the document
	XMLDocument document;
	XMLError error = document.LoadFile(filePath.c_str());
	GUARANTEE_OR_DIE(error == tinyxml2::XML_SUCCESS, Stringf("Error: BoardStateDefinition::LoadDefinitions() couldn't load file \"%s\"", filePath.c_str()));

	const XMLElement* rootElement = document.RootElement();
	const XMLElement* defElement = rootElement->FirstChildElement();

	while (defElement != nullptr)
	{
		BoardStateDefinition* definition = new BoardStateDefinition(*defElement);
		AddDefinition(definition);

		defElement = defElement->NextSiblingElement();
	}

	// Done loading all definitions, now register commands for console
	Command::Register("load_boardstate", "Loads a new boardstate of the given definition name", Command_LoadBoardState);
}


//-----------------------------------------------------------------------------------------------
// Adds the given definition to the registry, throwing a warning if the definition with the given
// name already exists
//
void BoardStateDefinition::AddDefinition(BoardStateDefinition* definition)
{
	std::string definitionName = definition->GetName();
	bool alreadyExists = (s_definitions.find(definitionName) != s_definitions.end());
	GUARANTEE_RECOVERABLE(!alreadyExists, Stringf("Warning: BoardStateDefinition::AddDefinition() attempted to add duplicate definition \"%s\"", definitionName.c_str()));

	s_definitions[definitionName] = definition;
}


//-----------------------------------------------------------------------------------------------
// Returns the definition given by definitionName, nullptr if no such definition exists
//
BoardStateDefinition* BoardStateDefinition::GetDefinition(const std::string& definitionName)
{
	bool definitionExists = (s_definitions.find(definitionName) != s_definitions.end());

	if (definitionExists)
	{
		return s_definitions.at(definitionName);
	}

	return nullptr;
}


//-----------------------------------------------------------------------------------------------
// Returns the map of definitions
//
const std::map<std::string, BoardStateDefinition*>& BoardStateDefinition::GetAllDefinitions()
{
	return s_definitions;
}


//-----------------------------------------------------------------------------------------------
// C Function used as a call back for a console command
// Returns to the main menu and loads the boardstate of the given name
//
void Command_LoadBoardState(Command& cmd)
{
	std::string boardStateDefName = cmd.GetNextString();

	BoardStateDefinition* definition = BoardStateDefinition::GetDefinition(boardStateDefName);

	if (definition == nullptr)
	{
		ConsoleErrorf("Error: Boardstate Definition \"%s\" doesn't exist.", boardStateDefName.c_str());
	}
	else
	{
		GameState_Playing* playState = new GameState_Playing(definition);
		Game::TransitionToGameState(playState);
		ConsolePrintf("Boardstate \"%s\" loaded successfully.", boardStateDefName.c_str());
	}
}
