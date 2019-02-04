/************************************************************************/
/* File: EntitySpawn_Default.hpp
/* Author: Andrew Chase
/* Date: February 3rd 2019
/* Description: Spawn event for just placing entities on the map
/************************************************************************/
#pragma once
#include "Game/Entity/EntitySpawn.hpp"

class EntitySpawnEvent_Default : public EntitySpawnEvent
{
public:
	//-----Public Methods-----

	virtual int RunSpawn() override;
	virtual EntitySpawnEvent* Clone(CampaignManager* manager) const override;


private:
	//-----Private Data-----


};
