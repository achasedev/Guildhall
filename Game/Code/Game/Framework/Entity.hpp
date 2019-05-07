/************************************************************************/
/* File: Entity.hpp
/* Author: Andrew Chase
/* Date: May 7th 2019
/* Description: Dummy class for testing NamedProperties with a custom type
/************************************************************************/
#pragma once
#include "Engine/Core/Utility/StringUtils.hpp"
#include <string>

class Entity
{
public:
	//-----Public Methods-----


public:
	//-----Public Data-----

	std::string m_name = "Andrew Chase";
	int m_age = 84893; // Really old
	float m_height = 7.0f; // You guess the units

};

// Need a ToString() method for it in order for it to work with templates
std::string ToString(Entity* entity);