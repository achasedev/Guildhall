/************************************************************************/
/* File: Portal.hpp
/* Author: Andrew Chase
/* Date: November 19th, 2017
/* Bugs: None
/* Description: Class to represent a spawned portal entity
/************************************************************************/
#pragma once
#include "Game/Entity.hpp"

class PortalDefinition;
class TileDefinition;
class Actor;


class Portal : public Entity
{
public:
	//-----Public Methods-----

	Portal(const Vector2& position, float orientation, const PortalDefinition* definition, Map* spawnMap, 
		const std::string& name, Map* destinationMap, const Vector2& destinationPosition);

	void Update(float deltaTime);
	void Render() const;

	void MoveActorToDestinationMap(Actor* actorToMove);

	// Accessors
	bool IsReady() const;
	Map* GetDestinationMap() const;

	// Mutators
	void SetIsReady(bool newState);


private:
	//-----Private Data-----

	Map* m_destinationMap;				// Map this portal teleports actors to
	Vector2 m_destinationPosition;		// Position on the destination map this portal teleports to
										
	bool m_isLocked = false;			// UNUSED - Flag to determine whether this portal is locked and requires an item/action to open
	bool m_isReady = false;				// Flag to signal when to start teleporting actors (to prevent infinite teleporting between portals)
};
