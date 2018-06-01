/************************************************************************/
/* File: ProjectileDefinition.hpp
/* Author: Andrew Chase
/* Date: November 18th, 2017
/* Bugs: None
/* Description: Class to define static data for a projectile entity
/************************************************************************/
#pragma once
#include <map>
#include <string>
#include "Game/EntityDefinition.hpp"
#include "Engine/Math/IntRange.hpp"

typedef tinyxml2::XMLElement XMLElement;


class ProjectileDefinition : public EntityDefinition
{
public:
	//-----Public Data-----

	ProjectileDefinition(const XMLElement& projectileDefElement);

	static void LoadDefinitions();
	static const ProjectileDefinition* GetDefinitionByName(const std::string& definitionName);
	static const ProjectileDefinition* ParseXMLAttribute(const XMLElement& element, const char* attributeName, ProjectileDefinition* defaultValue);

	float GetMaxAge() const;


private:
	//-----Private Data-----

	// Static list of definitions loaded from file
	static std::map<std::string, ProjectileDefinition*> s_definitions;
	IntRange m_damageRange;												// Damage range this projectile inflicts on a hostile target
	float m_knockbackMagnitude = 0.f;									// Amount of knockback this projectile applies to entities on impact
	float m_maxAge = -1.f;
};
