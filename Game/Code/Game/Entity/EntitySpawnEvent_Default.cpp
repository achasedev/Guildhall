#include "Game/Entity/EntitySpawnEvent_Default.hpp"


//-----------------------------------------------------------------------------------------------
// Spawns an entity on the map immediately at a random location
//
int EntitySpawnEvent_Default::RunSpawn()
{
	Vector3 position = Vector3(128.f, 20.f, 128.f);
	float orientation = 90.f;

	SpawnEntity(position, orientation);
}
