/************************************************************************/
/* File: EntitySpawn.hpp
/* Author: Andrew Chase
/* Date: November 25th 2018
/* Description: Shared struct to describe an entity spawn
/************************************************************************/
#pragma once
#include "Engine/Math/Vector3.hpp"
class EntityDefinition;

struct EntitySpawn_t
{
	const EntityDefinition* definition = nullptr;
	Vector3 position;
	float orientation;
};
