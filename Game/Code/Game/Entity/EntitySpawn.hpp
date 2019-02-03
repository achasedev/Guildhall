/************************************************************************/
/* File: EntitySpawn.hpp
/* Author: Andrew Chase
/* Date: November 25th 2018
/* Description: Source file for different spawn specifications
/************************************************************************/
#pragma once
#include <vector>
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/IntAABB2.hpp"
#include "Engine/Math/IntRange.hpp"
#include "Engine/Core/Utility/XmlUtilities.hpp"

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
	Vector3 position;
	float orientation;
};


//-----------------------------------------------------------------------------------------------
// For Spawning during a playing stage for waves
//

enum eSpawnEventType
{
	SPAWN_EVENT_FALL,
	SPAWN_EVENT_RISE,
	SPAWN_EVENT_WALK_IN
};

class EntitySpawnEvent
{
public:
	//-----Public Methods

	virtual int					RunSpawn();
	virtual EntitySpawnEvent*	Clone(CampaignManager* manager) const;

	// Helper functions during spawning
	int							GetEntityCountLeftToSpawn() const;
	int							GetEntityCountSpawnedSoFar() const;
	int							GetLiveEntityCount() const;
	bool						IsFinished() const;


protected:
	//-----Protected Methods-----

	EntitySpawnEvent() {}
	EntitySpawnEvent(const XMLElement& spawnElement);


protected:
	//-----Protected Data-----

	// State
	int						m_numberSpawnedSoFar = 0;
	std::vector<Entity*>	m_entitiesCurrentAliveFromThisEvent;

	// Data
	CampaignManager*		m_manager = nullptr;
	const EntityDefinition* definition = nullptr;	// What to spawn
	int						totalToSpawn = 0;		// Total amount to spawn in this event
	int						spawnRate = 1;			// The number of entities to spawn per spawn tick
	int						spawnCountDelay = 0;	// How many entities should spawn in the stage before this event should start
	float					spawnTimeDelay = 0.f;	// How long to wait in time before this event should start
	eSpawnEventType			type;					// Subclass of this spawn event, determining how they spawn in

};