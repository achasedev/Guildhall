#include "Game/Entity/Swarmer.hpp"
#include "Game/Framework/Game.hpp"
#include "Game/Environment/Map.hpp"
#include "Game/Entity/SwarmerSpawner.hpp"

SwarmerSpawner::SwarmerSpawner(const Vector3& position, unsigned int team)
	: Spawner(position, team)
{
	SetSpawnRatePerMinute(120.f);
}


//-----------------------------------------------------------------------------------------------
// Spawns an NPC tank
//
void SwarmerSpawner::SpawnEntity() const
{
	Swarmer* swarmer = new Swarmer(transform.position, m_team);

	Game::GetMap()->AddGameEntity(swarmer);
}
