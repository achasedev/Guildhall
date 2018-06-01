/************************************************************************/
/* File: MapGenStep.cpp
/* Author: Andrew Chase
/* Date: November 2nd, 2017
/* Bugs: None
/* Description: Implementation of the MapGenStep class
/************************************************************************/
#include "Game/MapGenStep.hpp"
#include "Game/MapGenStep_FillAndEdge.hpp"
#include "Game/MapGenStep_Mutate.hpp"
#include "Game/MapGenStep_FromFile.hpp"
#include "Game/MapGenStep_SpawnActor.hpp"
#include "Game/MapGenStep_SpawnItem.hpp"
#include "Game/MapGenStep_CellularAutomata.hpp"
#include "Game/MapGenStep_RoomsAndPaths.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include  "Engine/Core/XmlUtilities.hpp"

//-----------------------------------------------------------------------------------------------
// Only constructor - just sets the name of this generation step to the name of the subclass
//
MapGenStep::MapGenStep(const tinyxml2::XMLElement& mapGenerationElement)
{
	m_name = mapGenerationElement.Name();
	m_chanceToRun = ParseXmlAttribute(mapGenerationElement, "chanceToRun", m_chanceToRun);
	m_iterationsRange = ParseXmlAttribute(mapGenerationElement, "iterations", m_iterationsRange);
}


//-----------------------------------------------------------------------------------------------
// Virtual Destructor for subclass destructors
//
MapGenStep::~MapGenStep()
{
	// Virtual for overrides
}


//-----------------------------------------------------------------------------------------------
// Factory function for creating a MapGenStep of a given subclass
//
MapGenStep* MapGenStep::CreateMapGenStep(const XMLElement& genStepElement)
{
	std::string genStepName = genStepElement.Name();

	if		(genStepName.compare("FillAndEdge") == 0)		{ return new MapGenStep_FillAndEdge(genStepElement); }
	else if (genStepName.compare("Mutate") == 0)			{ return new MapGenStep_Mutate(genStepElement); }
	else if (genStepName.compare("FromFile") == 0)			{ return new MapGenStep_FromFile(genStepElement); }
	else if (genStepName.compare("SpawnActor") == 0)		{ return new MapGenStep_SpawnActor(genStepElement); }
	else if (genStepName.compare("SpawnItem") == 0)			{ return new MapGenStep_SpawnItem(genStepElement); }
	else if (genStepName.compare("CellularAutomata") == 0)	{ return new MapGenStep_CellularAutomata(genStepElement); }
	else if (genStepName.compare("RoomsAndPaths") == 0)		{ return new MapGenStep_RoomsAndPaths(genStepElement); }

	else { ERROR_AND_DIE(Stringf("Error: Couldn't create MapGenStep of subclass \"%s\"", genStepName.c_str()));}	
}
