/************************************************************************/
/* File: EntitySpawn.hpp
/* Author: Andrew Chase
/* Date: November 25th 2018
/* Description: Shared struct to describe an entity spawn
/************************************************************************/
#pragma once
#include <vector>
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/IntAABB2.hpp"
#include "Engine/Math/IntRange.hpp"

class EntityDefinition;

struct EntitySpawnArea_t
{
public:
	//-----Public Methods-----


public:
	//-----Public Data-----

	const EntityDefinition* m_definitionToSpawn = nullptr;
	bool m_allowOverlapsInThisArea = false;
	bool m_allowOverlapsGlobally = false;

	IntRange m_countRangeToSpawn = IntRange(1, 1);

	IntAABB2 m_spawnBounds = IntAABB2::ZEROS; 

	float m_spawnOrientation = -1.f; // -1 here indicates use random orientations

};
