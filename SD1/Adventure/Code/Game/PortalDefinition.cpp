/************************************************************************/
/* File: PortalDefinition.cpp
/* Author: Andrew Chase
/* Date: November 19th, 2017
/* Bugs: None
/* Description: Implementation of the PortalDefinition class
/************************************************************************/
#include "Game/PortalDefinition.hpp"
#include "Engine/Core/XmlUtilities.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

// Static map of all actor definitions loaded
std::map<std::string, PortalDefinition*> PortalDefinition::s_definitions;


//-----------------------------------------------------------------------------------------------
// Constructs a PortalDefinition given a PortalDefintion XMLElement
//
PortalDefinition::PortalDefinition(const XMLElement& portalDefElement)
	: EntityDefinition(portalDefElement)
{
	// Parse portal information here
}


//-----------------------------------------------------------------------------------------------
// Loads the PortalDefinition XML file and constructs all PortalDefinitions
//
void PortalDefinition::LoadDefinitions()
{
	// Load the XML file, and parse the root-child elements for tile definitions
	tinyxml2::XMLDocument adventureDefinitionDocument;
	adventureDefinitionDocument.LoadFile("Data/Definitions/PortalDefinition.xml");

	tinyxml2::XMLElement* currDefinitionElement = adventureDefinitionDocument.RootElement()->FirstChildElement();

	while (currDefinitionElement != nullptr)
	{
		PortalDefinition* newDefinition = new PortalDefinition(*currDefinitionElement);

		// Safety check - no duplicate definitions
		bool definitionAlreadyMade = (s_definitions.find(newDefinition->GetName()) != s_definitions.end());
		GUARANTEE_OR_DIE(!definitionAlreadyMade, Stringf("Error: Duplicate Portal definition in PortalDefinition.xml - \"%s\"", newDefinition->GetName().c_str()));

		// Add the definition to the map
		s_definitions[newDefinition->GetName()] = newDefinition;

		// Move to the next element
		currDefinitionElement = currDefinitionElement->NextSiblingElement();
	}
}


//-----------------------------------------------------------------------------------------------
// Parses the given XML element for the given attribute and interprets it as the name of a PortalDefinition,
// returning the corresponding PortalDefinition* from the static array
//
const PortalDefinition* PortalDefinition::ParseXMLAttribute(const XMLElement& element, const char* attributeName, PortalDefinition* defaultValue)
{
	std::string defaultPortalName = ParseXmlAttribute(element, attributeName, nullptr);

	if (defaultPortalName.compare(std::string("")) == 0)
	{
		return defaultValue;
	}

	// Safety check - ensure the definition exists
	bool definitionExists = (s_definitions.find(defaultPortalName) != s_definitions.end());
	GUARANTEE_OR_DIE(definitionExists, Stringf("Error: PortalDefinition::ParseXMLAttribute does not have PortalDefinition \"%s\".", defaultPortalName.c_str()));

	return s_definitions[defaultPortalName];
}
