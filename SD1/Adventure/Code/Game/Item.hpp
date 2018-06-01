/************************************************************************/
/* File: Item.hpp
/* Author: Andrew Chase
/* Date: November 28th, 2017
/* Bugs: None
/* Description: Class to represent a spawned item in the game
/************************************************************************/
#pragma once
#include "Game/Entity.hpp"
#include "Game/ItemDefinition.hpp"

class ItemDefinition;
class Actor;

class Item : public Entity
{
public:
	//-----Public Methods-----

	Item(const Vector2& position, float orientation, const ItemDefinition* definition, Map* entityMap, const std::string& name, Actor* actor=nullptr);

	virtual void Update(float deltaTime) override;
	virtual void Render() const override;

	// Mutators
	void SetActorHoldingThis(Actor* actorHoldingThisItem);

	// Accessors
	EquipSlot	GetEquipSlot() const;


private:
	//-----Private Methods

	void UpdateAnimationFromActor(float deltaTime);
	void RenderAnimationFromActor() const;

private:
	//-----Private Data-----

	const ItemDefinition* m_itemDefinition;	// The definition this item is associated with
	Actor* m_actorHoldingItem;				// The actor that is holding/has item equipped, nullptr if no actor
};
