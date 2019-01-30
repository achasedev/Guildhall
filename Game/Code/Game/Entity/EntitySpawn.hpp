/************************************************************************/
/* File: EntitySpawn.hpp
/* Author: Andrew Chase
/* Date: November 25th 2018
/* Description: Shared struct to describe an entity spawn
/************************************************************************/
#pragma once
#include <vector>
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/IntVector2.hpp"

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

	int m_spawnCount = 1;

	IntVector2 m_spawnBoundsMins = IntVector2::ZERO;
	IntVector2 m_spawnBoundsMaxs = IntVector2::ZERO;

	float m_spawnOrientation = -1.f; // -1 here indicates use random orientations

};
