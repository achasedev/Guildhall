/************************************************************************/
/* File: EntitySpawnEvent_Meteor.hpp
/* Author: Andrew Chase
/* Date: February 12dth 2019
/* Description: Class for the event where enemies spawn out of a meteor
/************************************************************************/
#pragma once
#include "Game/Entity/EntitySpawn.hpp"
#include "Engine/Core/Time/Stopwatch.hpp"


class EntitySpawnEvent_Meteor : public EntitySpawnEvent
{
public:
	//-----Public Methods-----

	EntitySpawnEvent_Meteor(const XMLElement& element);

	virtual void				Update() override;
	virtual int					RunSpawn(int maxAmountAllowedToSpawn) override;
	virtual EntitySpawnEvent*	Clone(CampaignManager* manager) const;


private:
	//-----Private Data-----

	bool m_meteorFinished = false;
	const EntityDefinition* m_definitionOfMeteorEntity = nullptr;
	Entity* m_meteorEntity = nullptr;

	Vector3 m_meteorStartPosition;
	Vector3 m_meteorTargetPosition;

	float m_meteorFallDuration = 1.0f;
	Stopwatch m_fallTimer;

};
