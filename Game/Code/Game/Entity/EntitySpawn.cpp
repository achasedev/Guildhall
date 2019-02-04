#include "Game/Framework/Game.hpp"
#include "Game/Framework/World.hpp"
#include "Game/Entity/EntitySpawn.hpp"
#include "Game/Framework/CampaignManager.hpp"
#include "Engine/Core/Utility/StringUtils.hpp"
#include "Engine/Core/Utility/ErrorWarningAssert.hpp"

//---C FUNCTION----------------------------------------------------------------------------------
// Returns the enumeration for the spawn type given by the text
//
eSpawnEventType GetSpawnTypeFromString(const std::string& text)
{
	if (text == "fall")			{ return SPAWN_EVENT_FALL; }
	else if (text == "rise")	{ return SPAWN_EVENT_RISE; }
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

	// Spawn details
	m_spawnRate = ParseXmlAttribute(spawnElement, "spawn_rate", m_spawnRate);
	m_spawnCountDelay = ParseXmlAttribute(spawnElement, "spawn_count_delay", m_spawnCountDelay);
	m_spawnTimeDelay = ParseXmlAttribute(spawnElement, "spawn_time_delay", m_spawnTimeDelay);
	m_totalToSpawn = ParseXmlAttribute(spawnElement, "total_to_spawn", m_totalToSpawn);
}


//-----------------------------------------------------------------------------------------------
// Spawns this event's entity, and updates the state tracking information
//
void EntitySpawnEvent::SpawnEntity(const Vector3& position, float orientation)
{
	// Spawn the entity
	World* world = Game::GetWorld();

	just spawn as ai entity
	Entity* entity = world->SpawnEntity(m_definitionToSpawn, position, orientation);

	// Update state tracking
	m_entitiesCurrentAliveFromThisEvent.push_back(entity);
	fdsafdasjklfajlkfdlj; kfasd;ljkasfd
}

