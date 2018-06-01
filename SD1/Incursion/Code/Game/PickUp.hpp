/************************************************************************/
/* Project: Incursion
/* File: PickUp.hpp
/* Author: Andrew Chase
/* Date: October 22nd, 2017
/* Bugs: None
/* Description: Class to represent a power up a player can pick up
/************************************************************************/
#pragma once
#include "Game/Entity.hpp"
#include "Engine/Math/IntVector2.hpp"

enum PickUpType
{
	PICKUP_TYPE_ERROR = -1,
	PICKUP_TYPE_HEALTH,
	PICKUP_TYPE_INVINCIBILITY,
	PICKUP_TYPE_REINFORCEMENTS,
	PICKUP_TYPE_SPEEDBOOST,
	NUM_PICKUP_TYPES
};


class PickUp : public Entity
{
public:

	PickUp(Vector2 spawnPosition, PickUpType pickUpType, Map* theMap);
	virtual ~PickUp() override;						// Used to player a sound effect on destruction

	virtual void Update(float deltaTime) override;	// Moves the PickUp in a circular fashion
	virtual void Render() const override;			// Draws the PickUp

	void ApplyPickUpEffectToPlayer();				// Consumes the pickup, marking it for destruction

private:

	IntVector2 m_spriteCoords;						// Which sprite in the PickUp spritesheet this PickUp will render
	PickUpType m_pickUpType;						// Specifies what this PickUp does to the player when picked up
	Vector2 m_anchorPosition;						// The position the PickUp oscillates back and forth over

	static const float PICKUP_DEFAULT_INNER_RADIUS;
	static const float PICKUP_DEFAULT_OUTER_RADIUS;
	static const float PICKUP_OSCILLATE_MAGNITUDE;		// Max distance from the anchor position the pickup will move
	static const float PICKUP_OSCILLATE_FREQUENCY;		// The speed the pickup will move at


};