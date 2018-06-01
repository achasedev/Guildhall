/************************************************************************/
/* File: Actor.hpp
/* Author: Andrew Chase
/* Date: November 15th, 2017
/* Bugs: None
/* Description: Class to represent spawned actors in the game
/************************************************************************/
#pragma once
#include <vector>
#include "Game/Entity.hpp"
#include "Game/EntityDefinition.hpp"
#include "Game/ItemDefinition.hpp"

class ActorDefinition;
class Item;
class Behavior;

class Actor : public Entity
{
public:
	//-----Public Methods-----

	Actor(const Vector2& position, float orientation, const ActorDefinition* definition, Map* entityMap, const std::string& name);
	~Actor();

	virtual void Update(float deltaTime) override;
	virtual void Render() const override;

	void EquipItem(Item* itemToEquip);
	void AddItemToInventory(Item* itemToAdd);
	void DropRandomEquippedItem();

	// Accessors
	virtual Stats	GetStats() const override;
	Stats			GetBaseStats() const;


protected:
	//-----Protected Methods-----

	void SetAnimationFromMovement();
	void SetIdleAnimation();
	void SetMovementAnimation();

	void UpdateEquippedItems(float deltaTime);
	void UpdateBehaviors(float deltaTime);
	void UpdateCurrentStats();

	void InitializeSpawnEquipment(const ActorDefinition* definition);
	void InitializeBehaviors(const ActorDefinition* definition);

	void RenderNameAndHealthBar() const;


protected:
	//-----Protected Data-----

	const ActorDefinition* m_actorDefinition;

	Stats m_currentStats;									// This actor's current stats, = base stats + equipped item stats
	Item* m_equippedItems[NUM_EQUIP_SLOTS];					// The items currently equipped by this actor
	std::vector<Item*> m_items;								// The items held by this entity, not including equipped items
	std::vector<Behavior*> m_behaviors;						// The behaviors this actor can adopt

};
