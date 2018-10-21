/************************************************************************/
/* File: SpawnPoint.hpp
/* Author: Andrew Chase
/* Date: October 20th 2018
/* Description: Class representing a source point for an entity spawn
/************************************************************************/
#pragma once
#include "Engine/Math/Vector3.hpp"
#include "Engine/Core/Utility/XmlUtilities.hpp"
#include <vector>

class AIEntity;
class EntityDefinition;

class SpawnPoint
{
public:
	//-----Public Methods-----
	
	SpawnPoint(const XMLElement& pointElement);
	~SpawnPoint();

	// Mutators
	void	SpawnEntity(const EntityDefinition* spawnType);
	void	StopTrackingEntity(const AIEntity* entity);

	// Accessors
	int		GetLiveSpawnCount() const;
	int		GetLiveSpawnCountForType(const EntityDefinition* type) const;

	
private:
	//-----Private Data-----

	float					m_radius = 0.f;
	Vector3					m_position = Vector3::ZERO;
	std::vector<AIEntity*>	m_spawnedEntities;

};
