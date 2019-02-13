/************************************************************************/
/* File: EntitySpawnEvent_FromGround.hpp
/* Author: Andrew Chase
/* Date: February 12th 2019
/* Description: Entity spawn event where enemies rise up from the ground
/************************************************************************/
#pragma once
#include "Game/Entity/EntitySpawn.hpp"
#include "Engine/Core/Time/Stopwatch.hpp"

struct RisingEntity_t
{
	int startingHeight = 0;
	AIEntity* entity = nullptr;
	Stopwatch riseTimer;
};


class EntitySpawnEvent_FromGround : public EntitySpawnEvent 
{
public:
	//-----Public Methods-----

	EntitySpawnEvent_FromGround(const XMLElement& element);

	virtual void				Update() override;
	virtual int					RunSpawn() override;
	virtual EntitySpawnEvent*	Clone(CampaignManager* manager) const override;


private:
	//-----Private Data-----

	float m_riseTime = 1.0f;

	std::vector<RisingEntity_t> m_entitiesRisingFromGround;

};
