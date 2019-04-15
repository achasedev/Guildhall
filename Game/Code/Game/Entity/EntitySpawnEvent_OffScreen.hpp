/************************************************************************/
/* File: EntitySpawnEvent_OffScreen.hpp
/* Author: Andrew Chase
/* Date: February 12th 2019
/* Description: Class for the spawn event where entities spawn off screen
/************************************************************************/
#pragma once
#include "Game/Entity/EntitySpawn.hpp"

class EntitySpawnEvent_OffScreen : public EntitySpawnEvent
{
public:
	//-----Public Methods-----

	EntitySpawnEvent_OffScreen(const XMLElement& element);

	virtual void				Update() override;
	virtual int					RunSpawn(int maxAmountAllowedToSpawn) override;
	virtual EntitySpawnEvent*	Clone(CampaignManager* manager) const override;


private:
	//-----Private Data-----

	std::string m_spawnDirection;

};
