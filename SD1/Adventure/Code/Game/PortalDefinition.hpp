/************************************************************************/
/* File: PortalDefinition.hpp
/* Author: Andrew Chase
/* Date: November 19th, 2017
/* Bugs: None
/* Description: Class to represent static Portal information from an XML file
/************************************************************************/
#pragma once
#include <map>
#include "Game/EntityDefinition.hpp"

typedef tinyxml2::XMLElement XMLElement;


class PortalDefinition : public EntityDefinition
{
public:
	//-----Public Methods-----

	PortalDefinition(const XMLElement& portalDefElement);

	static void LoadDefinitions();
	static const PortalDefinition* ParseXMLAttribute(const XMLElement& element, const char* attributeName, PortalDefinition* defaultValue);


private:
	//-----Private Data-----

	// Static list of definitions loaded from file
	static std::map<std::string, PortalDefinition*> s_definitions;

};
