/************************************************************************/
/* File: SpawnPoint.cpp
/* Author: Andrew Chase
/* Date: October 20th 2018
/* Description: Implementation of the SpawnPoint class
/************************************************************************/
#include "Game/Framework/Game.hpp"
#include "Game/Entity/AIEntity.hpp"
#include "Game/Framework/World.hpp"
#include "Game/Framework/SpawnPoint.hpp"
#include "Engine/Math/MathUtils.hpp"


//-----------------------------------------------------------------------------------------------
// Constructor
//
SpawnPoint::SpawnPoint(const XMLElement& pointElement)
{
	m_position = ParseXmlAttribute(pointElement, "position", m_position);
	m_radius = ParseXmlAttribute(pointElement, "radius", m_radius);
}


//-----------------------------------------------------------------------------------------------
// Destructor
//
SpawnPoint::~SpawnPoint()
{

}


//-----------------------------------------------------------------------------------------------
// Spawns an entity of the given type, generating a position from the point's position settings
//
void SpawnPoint::SpawnEntity(const EntityDefinition* spawnType)
{
	Vector2 offset = Vector2::GetRandomVector(1.0f) * GetRandomFloatInRange(0.f, m_radius);
	Vector3 spawnPosition = m_position + Vector3(offset.x, 0.f, offset.y);

	AIEntity* entity = new AIEntity(spawnType);
	entity->SetPosition(spawnPosition);
	entity->SetSpawnPoint(this);
	entity->SetTeam(ENTITY_TEAM_ENEMY);

	m_spawnedEntities.push_back(entity);
	Game::GetWorld()->AddEntity(entity);
}


//-----------------------------------------------------------------------------------------------
// Returns the number of live entities spawned from this spawn point
//
int SpawnPoint::GetLiveSpawnCount() const
{
	return (int) m_spawnedEntities.size();
}


//-----------------------------------------------------------------------------------------------
// Returns the number of live entities of the given type spawned from this spawn point 
//
int SpawnPoint::GetLiveSpawnCountForType(const EntityDefinition* type) const
{
	int numEntities = (int)m_spawnedEntities.size();

	int total = 0;
	for (int i = 0; i < numEntities; ++i)
	{
		if (m_spawnedEntities[i]->GetEntityDefinition() == type)
		{
			total++;
		}
	}

	return total;
}


//-----------------------------------------------------------------------------------------------
// Removes the given entity from this point's list of tracked entities
//
void SpawnPoint::StopTrackingEntity(const AIEntity* entity)
{
	int numEntities = (int) m_spawnedEntities.size();

	for (int i = 0; i < numEntities; ++i)
	{
		if (m_spawnedEntities[i] == entity)
		{
			m_spawnedEntities.erase(m_spawnedEntities.begin() + i);
			break;
		}
	}
}
