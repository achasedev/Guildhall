#pragma once
#include "Game/Entity/Spawner.hpp"

class TankSpawner : public Spawner
{
public:
	//-----Public Methods-----

	TankSpawner(const Vector3& position, unsigned int team);

	virtual void SpawnEntity() const override;


private:
	//-----Privaet Data-----



};
