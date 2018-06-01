/************************************************************************/
/* File: ProjectileDefinition.cpp
/* Author: Andrew Chase
/* Date: November 18th, 2017
/* Bugs: None
/* Description: Implementation of the ProjectileDefinition class
/************************************************************************/
#include "Game/ProjectileDefinition.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/XmlUtilities.hpp"


// Static map of all projectile definitions loaded
std::map<std::string, ProjectileDefinition*> ProjectileDefinition::s_definitions;


//-----------------------------------------------------------------------------------------------
// Constructs a ProjectileDefinition from an XMLElement
//
ProjectileDefinition::ProjectileDefinition(const XMLElement& projectileDefElement)
	: EntityDefinition(projectileDefElement)
{
	m_maxAge = ParseXmlAttribute(projectileDefElement, "maxAge", m_maxAge);
}


//-----------------------------------------------------------------------------------------------
// Loads all Projectile Definitions from an XMLFile through parsing
//
void ProjectileDefinition::LoadDefinitions()
{
	// Load the XML file, and parse the root-child elements for tile definitions
	tinyxml2::XMLDocument projectileDefinitionDocument;
	projectileDefinitionDocument.LoadFile("Data/Definitions/ProjectileDefinition.xml");

	XMLElement* currDefinitionElement = projectileDefinitionDocument.RootElement()->FirstChildElement();

	while (currDefinitionElement != nullptr)
	{
		ProjectileDefinition* newDefinition = new ProjectileDefinition(*currDefinitionElement);

		// Safety check - no duplicate definitions
		bool definitionAlreadyMade = (s_definitions.find(newDefinition->GetName()) != s_definitions.end());
		GUARANTEE_OR_DIE(!definitionAlreadyMade, Stringf("Error: Duplicate Projectile definition in ProjectileDefinition.xml - \"%s\"", newDefinition->GetName().c_str()));

		// Add the definition to the map
		s_definitions[newDefinition->GetName()] = newDefinition;

		// Move to the next element
		currDefinitionElement = currDefinitionElement->NextSiblingElement();
	}
}


//-----------------------------------------------------------------------------------------------
// Returns the ProjectileDefinition associated by the name definitionName
//
const ProjectileDefinition* ProjectileDefinition::GetDefinitionByName(const std::string& definitionName)
{
	// Safety check - ensure the definition exists
	bool definitionExists = (s_definitions.find(definitionName) != s_definitions.end());
	GUARANTEE_OR_DIE(definitionExists, Stringf("Error: ProjectileDefinition::GetDefinitionByName - \"%s\" does not exist.", definitionName.c_str()));

	return s_definitions[definitionName];
}


//-----------------------------------------------------------------------------------------------
// Parses the given element for the given attribute and interprets it as a projectile definition
//
const ProjectileDefinition* ProjectileDefinition::ParseXMLAttribute(const XMLElement& element, const char* attributeName, ProjectileDefinition* defaultValue)
{
	std::string defaultProjectileName = ParseXmlAttribute(element, attributeName, nullptr);

	if (defaultProjectileName.compare(std::string("")) == 0)
	{
		return defaultValue;
	}

	// Safety check - ensure the definition exists
	bool definitionExists = (s_definitions.find(defaultProjectileName) != s_definitions.end());
	GUARANTEE_OR_DIE(definitionExists, Stringf("Error: ProjectileDefinition::ParseXMLAttribute does not have ProjectileDefinition \"%s\".", defaultProjectileName.c_str()));

	return s_definitions[defaultProjectileName];
}


//-----------------------------------------------------------------------------------------------
// Returns the max age projectiles of this age can live for
//
float ProjectileDefinition::GetMaxAge() const
{
	return m_maxAge;
}
