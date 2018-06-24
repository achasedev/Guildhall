#pragma once
#include "Game/Entity/Spawner.hpp"

class SwarmerSpawner : public Spawner
{
public:
	//-----Public Methods-----

	SwarmerSpawner(const Vector3& position, unsigned int team);

	virtual void SpawnEntity() const override;


private:
	//-----Private Data-----



};
