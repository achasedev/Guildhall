/************************************************************************/
/* File: Entity.cpp
/* Author: Andrew Chase
/* Date: May 7th 2019
/* Description: Implementation of the Entity class
/************************************************************************/
#include "Game/Framework/Entity.hpp"

std::string ToString(Entity* entity)
{
	return Stringf("Name: %s\nAge: %i\nHeight: %.2f", entity->m_name.c_str(), entity->m_age, entity->m_height);
}