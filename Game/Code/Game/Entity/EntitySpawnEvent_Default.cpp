#include "Game/Entity/EntitySpawnEvent_Default.hpp"


//-----------------------------------------------------------------------------------------------
// Constructor from XML
//
EntitySpawnEvent_Default::EntitySpawnEvent_Default(const XMLElement& spawnElement)
	: EntitySpawnEvent(spawnElement)
{
	// Parse information here
}


//-----------------------------------------------------------------------------------------------
// Update
//
void EntitySpawnEvent_Default::Update()
{
}


//-----------------------------------------------------------------------------------------------
// Spawns an entity on the map immediately at a random location
//
int EntitySpawnEvent_Default::RunSpawn()
{
	Vector3 position = Vector3(128.f, 20.f, 128.f);
	float orientation = 90.f;

	SpawnEntity(position, orientation);

	return 1; // only spawned 1
}


//-----------------------------------------------------------------------------------------------
// Clone, for creating running events from a prototype
//
EntitySpawnEvent* EntitySpawnEvent_Default::Clone(CampaignManager* manager) const
{
	EntitySpawnEvent_Default* defaultEvent = new EntitySpawnEvent_Default(*this);
	defaultEvent->m_manager = manager;

	return defaultEvent;
}
