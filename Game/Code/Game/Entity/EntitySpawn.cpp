#include "Game/Framework/Game.hpp"
#include "Game/Framework/World.hpp"
#include "Game/Entity/AIEntity.hpp"
#include "Game/Entity/EntitySpawn.hpp"
#include "Game/Framework/CampaignManager.hpp"
#include "Engine/Core/Utility/StringUtils.hpp"
#include "Game/Entity/EntitySpawnEvent_Meteor.hpp"
#include "Game/Entity/EntitySpawnEvent_Default.hpp"
#include "Engine/Core/Utility/ErrorWarningAssert.hpp"
#include "Game/Entity/EntitySpawnEvent_FromGround.hpp"


//---C FUNCTION----------------------------------------------------------------------------------
// Returns the enumeration for the spawn type given by the text
//
eSpawnEventType GetSpawnTypeFromString(const std::string& text)
{
	if (text == "meteor")			{ return SPAWN_EVENT_METEOR; }
	else if (text == "from_ground")	{ return SPAWN_EVENT_RISE; }
	else
	{
		return SPAWN_EVENT_DEFAULT;
	}
}


//-----------------------------------------------------------------------------------------------
// Returns the number of entities this event still needs to spawn
//
int EntitySpawnEvent::GetEntityCountLeftToSpawn() const
{
	return (m_totalToSpawn - m_entityCountSpawnedSoFar);
}


//-----------------------------------------------------------------------------------------------
// Returns the number of entities this event has spawned so far
//
int EntitySpawnEvent::GetEntityCountSpawnedSoFar() const
{
	return m_entityCountSpawnedSoFar;
}


//-----------------------------------------------------------------------------------------------
// Returns the number of entities currently alive in the world that are from this event
//
int EntitySpawnEvent::GetLiveEntityCount() const
{
	return (int)m_entitiesCurrentAliveFromThisEvent.size();
}


//-----------------------------------------------------------------------------------------------
// Returns whether this event is finished spawning new entities
//
bool EntitySpawnEvent::IsFinishedSpawning() const
{
	return (GetEntityCountLeftToSpawn() == 0);
}


//-----------------------------------------------------------------------------------------------
// Returns true if this event should spawn this tick based on its spawn parameters
//
bool EntitySpawnEvent::IsReadyForNextSpawn() const
{
	// Time check
	float timeIntoStage = m_manager->GetTimeIntoStage();
	bool passedTimeCheck = timeIntoStage >= m_spawnTimeDelay;

	// Count check
	int countSpawnedInStage = m_manager->GetEntityCountSpawnedThisStageSoFar();
	bool passedCountCheck = countSpawnedInStage >= m_spawnCountDelay;

	return (passedTimeCheck && passedCountCheck);
}


//-----------------------------------------------------------------------------------------------
// Stops tracking the given entity by removing it from its tracking list
//
void EntitySpawnEvent::StopTrackingEntity(AIEntity* entity)
{
	int numEntities = (int)m_entitiesCurrentAliveFromThisEvent.size();
	bool found = false;

	for (int entityIndex = 0; entityIndex < numEntities; ++entityIndex)
	{
		if (m_entitiesCurrentAliveFromThisEvent[entityIndex] == entity)
		{
			m_entitiesCurrentAliveFromThisEvent.erase(m_entitiesCurrentAliveFromThisEvent.begin() + entityIndex);
			found = true;
		}
	}

	ASSERT_OR_DIE(found, "Event asked to stop tracking entity when it wasn't being tracked");
}


//-----------------------------------------------------------------------------------------------
// Returns true if the given entity is in this event's tracking list
//
bool EntitySpawnEvent::IsEventTrackingThisEntity(AIEntity* entity)
{
	int numEntities = (int)m_entitiesCurrentAliveFromThisEvent.size();

	for (int entityIndex = 0; entityIndex < numEntities; ++entityIndex)
	{
		if (m_entitiesCurrentAliveFromThisEvent[entityIndex] == entity)
		{
			return true;
		}
	}

	return false;
}


//-----------------------------------------------------------------------------------------------
// Creates a spawn event prototype of the correct subclass given the type
//
EntitySpawnEvent* EntitySpawnEvent::CreateSpawnEventForElement(const XMLElement& element)
{
	// Type subclass
	std::string typeText = ParseXmlAttribute(element, "type", "default");
	eSpawnEventType type = GetSpawnTypeFromString(typeText);

	switch (type)
	{
	case SPAWN_EVENT_DEFAULT:
		return new EntitySpawnEvent_Default(element);
		break;
	case SPAWN_EVENT_RISE:
		return new EntitySpawnEvent_FromGround(element);
		break;
	case SPAWN_EVENT_METEOR:
		return new EntitySpawnEvent_Meteor(element);
		break;
	default:
		ERROR_AND_DIE(Stringf("Unsupported spawn event type attempted to be created: \"%s\"", typeText.c_str()).c_str());
		break;
	}
}


//-----------------------------------------------------------------------------------------------
// Constructor from an XML Element
//
EntitySpawnEvent::EntitySpawnEvent(const XMLElement& spawnElement)
{
	// Type subclass
	std::string typeText = ParseXmlAttribute(spawnElement, "type", "default");
	m_type = GetSpawnTypeFromString(typeText);

	// Entity
	std::string entityDefName = ParseXmlAttribute(spawnElement, "entity", "");
	GUARANTEE_OR_DIE(!IsStringNullOrEmpty(entityDefName), "No entity specified in spawn event element");

	m_definitionToSpawn = EntityDefinition::GetDefinition(entityDefName);
	GUARANTEE_OR_DIE(m_definitionToSpawn->m_entityClass == ENTITY_CLASS_AI, Stringf("Spawn Event initialized by non-AI definition \"%s\"", m_definitionToSpawn->m_name.c_str()).c_str());

	// Spawn details
	m_spawnRate = ParseXmlAttribute(spawnElement, "spawn_rate", m_spawnRate);
	m_spawnCountDelay = ParseXmlAttribute(spawnElement, "spawn_count_delay", m_spawnCountDelay);
	m_spawnTimeDelay = ParseXmlAttribute(spawnElement, "spawn_time_delay", m_spawnTimeDelay);
	m_totalToSpawn = ParseXmlAttribute(spawnElement, "total_to_spawn", m_totalToSpawn);

	m_areaToSpawnIn.mins = ParseXmlAttribute(spawnElement, "spawn_mins", m_areaToSpawnIn.mins);
	m_areaToSpawnIn.maxs = ParseXmlAttribute(spawnElement, "spawn_maxs", m_areaToSpawnIn.maxs);
}


//-----------------------------------------------------------------------------------------------
// Spawns this event's entity, and updates the state tracking information
//
AIEntity* EntitySpawnEvent::SpawnEntity(const Vector3& position, float orientation)
{
	// Spawn the entity
	World* world = Game::GetWorld();

	AIEntity* entity = new AIEntity(m_definitionToSpawn);
	entity->SetPosition(position);
	entity->SetOrientation(orientation);
	entity->SetSpawnEvent(this);
	entity->SetTeam(ENTITY_TEAM_ENEMY);

	world->AddEntity(entity);

	// Update state tracking
	m_entitiesCurrentAliveFromThisEvent.push_back(entity);
	m_entityCountSpawnedSoFar++;

	return entity;
}
