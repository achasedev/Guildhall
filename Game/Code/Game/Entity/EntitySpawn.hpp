/************************************************************************/
/* File: EntitySpawn.hpp
/* Author: Andrew Chase
/* Date: November 25th 2018
/* Description: Source file for different spawn specifications
/************************************************************************/
#pragma once
#include <vector>
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/IntAABB2.hpp"
#include "Engine/Math/IntRange.hpp"
#include "Engine/Core/Utility/XmlUtilities.hpp"

class Entity;
class AIEntity;
class CampaignManager;
class EntityDefinition;

//-----------------------------------------------------------------------------------------------
// For spawning on maps when they're generated
// 
struct MapAreaSpawn_t
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


//-----------------------------------------------------------------------------------------------
// For spawning in a stage when the stage is initialized
// 
struct InitialStageSpawn_t
{
	const EntityDefinition* definition = nullptr;
	Vector2 position; // Map XZ only, will determine the height when spawned
	float orientation;
};


//-----------------------------------------------------------------------------------------------
// For Spawning AI enemies (and other teams?) during a playing stage for waves
//

enum eSpawnEventType
{
	SPAWN_EVENT_DEFAULT,
	SPAWN_EVENT_FALL,
	SPAWN_EVENT_RISE,
	SPAWN_EVENT_WALK_IN
};

class EntitySpawnEvent
{
public:
	//-----Public Methods

	virtual void				Update() = 0;
	virtual int					RunSpawn() = 0;
	virtual EntitySpawnEvent*	Clone(CampaignManager* manager) const = 0;

	// Helper functions during spawning
	int							GetEntityCountLeftToSpawn() const;
	int							GetEntityCountSpawnedSoFar() const;
	int							GetLiveEntityCount() const;
	bool						IsFinishedSpawning() const;
	bool						IsReadyForNextSpawn() const;

	void						StopTrackingEntity(AIEntity* entity);
	bool						IsEventTrackingThisEntity(AIEntity* entity);

	static EntitySpawnEvent*	CreateSpawnEventForElement(const XMLElement& element);


protected:
	//-----Protected Methods-----

	EntitySpawnEvent(const XMLElement& spawnElement);

	AIEntity*					SpawnEntity(const Vector3& position, float orientation);


protected:
	//-----Protected Data-----

	// State
	int						m_entityCountSpawnedSoFar = 0;
	std::vector<AIEntity*>	m_entitiesCurrentAliveFromThisEvent;

	// Data
	CampaignManager*		m_manager = nullptr;
	const EntityDefinition* m_definitionToSpawn = nullptr;		// What to spawn
	int						m_totalToSpawn = 0;					// Total amount to spawn in this event
	int						m_spawnRate = 1;					// The number of entities to spawn per spawn tick
	int						m_spawnCountDelay = 0;				// How many entities should spawn in the stage before this event should start
	float					m_spawnTimeDelay = 0.f;				// How long to wait in time before this event should start
	eSpawnEventType			m_type;								// Subclass of this spawn event, determining how they spawn in

	AABB2					m_areaToSpawnIn = AABB2(Vector2::ZERO, Vector2(256.f));		// The area that the entities should be spawned in

};
