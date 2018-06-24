#include "Game/Entity/NPCTank.hpp"
#include "Game/Framework/Game.hpp"
#include "Game/Environment/Map.hpp"
#include "Game/Entity/TankSpawner.hpp"

TankSpawner::TankSpawner(const Vector3& position, unsigned int team)
	: Spawner(position, team)
{
}

//-----------------------------------------------------------------------------------------------
// Spawns an NPC tank
//
void TankSpawner::SpawnEntity() const
{
	NPCTank* tank = new NPCTank(m_team);
	tank->transform = transform;

	Game::GetMap()->AddGameEntity(tank);
}
