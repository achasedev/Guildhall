/************************************************************************/
/* File: Actor.hpp
/* Author: Andrew Chase
/* Date: November 15th, 2017
/* Bugs: None
/* Description: Implementation of the Actor class
/************************************************************************/
#include "Game/Actor.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Player.hpp"
#include "Game/ActorDefinition.hpp"
#include "Game/ProjectileDefinition.hpp"
#include "Game/Map.hpp"
#include "Game/Item.hpp"
#include "Game/Behavior.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/SpriteAnimSet.hpp"
#include "Engine/Renderer/SpriteAnim.hpp"
#include "Engine/Math/MathUtils.hpp"


//-----------------------------------------------------------------------------------------------
// Constructs an actor given the actor information
//
Actor::Actor(const Vector2& position, float orientation, const ActorDefinition* definition, Map* entityMap, const std::string& name)
	: Entity(position, orientation, definition, entityMap, name)
	, m_actorDefinition(definition)
{
	InitializeSpawnEquipment(definition);
	InitializeBehaviors(definition);
}


//-----------------------------------------------------------------------------------------------
// On actor deletion - if the actor is on a map, have them drop a random equipped item
//
Actor::~Actor()
{
	if (m_map != nullptr && !m_map->IsBeingDeleted())
	{
		DropRandomEquippedItem();
	}

	// Delete all equipped items
	for (int equipIndex = 0; equipIndex < static_cast<int>(NUM_EQUIP_SLOTS); equipIndex++)
	{
		delete m_equippedItems[equipIndex];
	}

	// Delete inventory
	for (int itemIndex = 0; itemIndex < static_cast<int>(m_items.size()); itemIndex++)
	{
		delete m_items[itemIndex];
	}

	// Delete behaviors
	for (int behaviorIndex = 0; behaviorIndex < static_cast<int>(m_behaviors.size()); behaviorIndex++)
	{
		delete m_behaviors[behaviorIndex];
	}
}


//-----------------------------------------------------------------------------------------------
// Updates the actor, called once per frame
//
void Actor::Update(float deltaTime)
{
	// If this actor is still alive
	if (!m_isMarkedForDeletion)
	{
		UpdateBehaviors(deltaTime);			// Update the actor's behavior
		SetAnimationFromMovement();			// Set the animation that should be played	
		UpdateEquippedItems(deltaTime);		// Update the equipped items
		UpdateCurrentStats();				// Update the actor's stats every frame
		Entity::Update(deltaTime);			// Update the entity's base state, including animation
	}	
}


//-----------------------------------------------------------------------------------------------
// Draws the actor to screen - by drawing the sprite animation
//
void Actor::Render() const
{
	// Rendering of the sprite animation is handled by the Entity class
	Entity::Render();

	// Draw the equipped items

	// Render the equipped items in local actor coordinates
	for (int i = 0; i < NUM_EQUIP_SLOTS; i++)
	{
		Item* currItem = m_equippedItems[i];
		if (currItem != nullptr)
		{
			currItem->Render();
		}
	}

	// Draw a healthbar and name over the actor
	RenderNameAndHealthBar();
}


//-----------------------------------------------------------------------------------------------
// Equips the given item to the player, dropping an already-equipped item in that spot to the actor's
// inventory
//
void Actor::EquipItem(Item* itemToEquip)
{
	EquipSlot itemEquipSlot = itemToEquip->GetEquipSlot();
	Item* itemAlreadyInSlot = m_equippedItems[itemEquipSlot];

	if (itemAlreadyInSlot != nullptr)
	{
		itemAlreadyInSlot->SetPosition(m_position);
		m_map->AddItemToMap(m_equippedItems[itemEquipSlot]);
	}
	
	m_equippedItems[itemEquipSlot] = itemToEquip;
	itemToEquip->PlayAnimation();
	itemToEquip->SetActorHoldingThis(this);
}


//-----------------------------------------------------------------------------------------------
// Adds the given item to the actor's inventory
//
void Actor::AddItemToInventory(Item* itemToAdd)
{
	m_items.push_back(itemToAdd);
}


//-----------------------------------------------------------------------------------------------
// Drops a random item from the actor's equipped item list at random
//
void Actor::DropRandomEquippedItem()
{
	if (m_map != nullptr)
	{
		int itemDropIndex = -1;
		float numEquippedItemsFoundSoFar = 0.f;

		// Find an equipped item at random
		for (int i = 0; i < NUM_EQUIP_SLOTS; i++)
		{
			if (m_equippedItems[i] != nullptr)
			{
				numEquippedItemsFoundSoFar += 1.f;
				float chanceToPick = 1.f / numEquippedItemsFoundSoFar;

				if  (CheckRandomChance(chanceToPick))
				{
					itemDropIndex = i;
				}
			}
		}

		if (itemDropIndex != -1)
		{
			Item* droppedItem = m_equippedItems[itemDropIndex];
			droppedItem->SetPosition(m_position);
			m_map->AddItemToMap(droppedItem);

			// Set the slot to null to prevent deleting the item just dropped
			m_equippedItems[itemDropIndex] = nullptr;
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Returns the current stats of this actor - overrides Entity::GetStats()
//
Stats Actor::GetStats() const
{
	return m_currentStats;
}


//-----------------------------------------------------------------------------------------------
// Returns the base stats of the actor
//
Stats Actor::GetBaseStats() const
{
	return m_baseStats;
}


//-----------------------------------------------------------------------------------------------
// Updates the actor's behavior based on the utility results
//
void Actor::UpdateBehaviors(float deltaTime)
{
	// Find the index of the max-utility behavior
	int maxIndex = -1;
	float maxUtility = 0.f;
	for (int behaviorIndex = 0; behaviorIndex < static_cast<int>(m_behaviors.size()); behaviorIndex++)
	{
		float currUtility = m_behaviors[behaviorIndex]->CalcUtility();
		if (currUtility > maxUtility)
		{
			maxUtility = currUtility;
			maxIndex = behaviorIndex;
		}
	}

	// If there was at least one behavior, we know we can update
	if (maxIndex != -1)
	{
		m_behaviors[maxIndex]->Update(deltaTime);
	}
}


//-----------------------------------------------------------------------------------------------
// Updates the Actor's current stats based on the equipment they are wearing
//
void Actor::UpdateCurrentStats()
{
	m_currentStats = m_baseStats;

	for (int equipIndex = 0; equipIndex < NUM_EQUIP_SLOTS; equipIndex++)
	{
		if (m_equippedItems[equipIndex] != nullptr)
		{
			Stats itemBonusStats = m_equippedItems[equipIndex]->GetStats();

			m_currentStats = m_currentStats + itemBonusStats;
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Creates and equips this actor's starting equipment as defined in its definition
//
void Actor::InitializeSpawnEquipment(const ActorDefinition* definition)
{
	for (int i = 0; i < NUM_EQUIP_SLOTS; i++)
	{
		const ItemDefinition* currItemDef = definition->m_spawnEquipment[i];

		if (currItemDef != nullptr)
		{
			m_equippedItems[i] = new Item(m_position, m_orientationDegrees, currItemDef, m_map, currItemDef->GetName(), this);
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Clones and initializes this actor's behaviors
//
void Actor::InitializeBehaviors(const ActorDefinition* definition)
{
	for (int behaviorIndex = 0; behaviorIndex < static_cast<int>(definition->m_behaviorNames.size()); behaviorIndex++)
	{
		std::string currBehaviorName = definition->m_behaviorNames[behaviorIndex];
		Behavior* newBehavior = Behavior::ClonePrototypeBehavior(currBehaviorName);
		newBehavior->SetActor(this);
		m_behaviors.push_back(newBehavior);
	}
}


//-----------------------------------------------------------------------------------------------
// Renders the actor's name and healtbar above their heads
//
void Actor::RenderNameAndHealthBar() const
{
	// Get the actor's draw bounds
	AABB2 localDrawBounds = GetLocalDrawBounds();

	// Set up coordinate system
	g_theRenderer->PushMatrix();
	g_theRenderer->TranslateCoordinateSystem2D(m_position);


	// Drawing the outline
	Vector2 outlineBottomLeft = Vector2(localDrawBounds.mins.x, localDrawBounds.maxs.y);
	Vector2 outlineTopRight = outlineBottomLeft + Vector2(localDrawBounds.GetDimensions().x, 0.3f);
	AABB2 outline = AABB2(outlineBottomLeft, outlineTopRight);
	g_theRenderer->DrawAABB2(outline, Rgba::BLACK);

	// Drawing the red backdrop
	Vector2 borderThickness = Vector2(0.05f, 0.05f);

	Vector2 redBarBottomLeft = outlineBottomLeft + borderThickness;
	Vector2 redBarTopRight		= outlineTopRight - borderThickness;
	g_theRenderer->DrawAABB2(AABB2(redBarBottomLeft, redBarTopRight), Rgba::RED);

	// Draw the green portion that represents remaining health
	float fractionHealthRemaining = static_cast<float>(m_health) / static_cast<float>(GetMaxHealth());
	float interiorBarLength = redBarTopRight.x - redBarBottomLeft.x;
	float greenBarLength = fractionHealthRemaining * interiorBarLength;

	Vector2 greenBarTopRight = Vector2(redBarBottomLeft.x + greenBarLength, redBarTopRight.y);

	g_theRenderer->DrawAABB2(AABB2(redBarBottomLeft, greenBarTopRight), Rgba::LIGHT_GREEN);

	// Draw the name of the actor overhead
	Vector2 textBoxBottomLeft = Vector2(outlineBottomLeft.x, outlineTopRight.y);
	Vector2 textBoxTopRight = Vector2(outlineTopRight.x, outlineTopRight.y + 0.2f);
	AABB2 textBox = AABB2(textBoxBottomLeft, textBoxTopRight);

	g_theRenderer->DrawTextInBox2D(m_name, textBox, Vector2(0.5f, 0.5f), textBox.GetDimensions().y, TEXT_DRAW_OVERRUN, nullptr, Rgba::BLACK);

	g_theRenderer->PopMatrix();
}


//-----------------------------------------------------------------------------------------------
// Updates this actor's animation based on their movement
//
void Actor::SetAnimationFromMovement()
{
	if (m_velocity == Vector2::ZERO)
	{
		SetIdleAnimation();
	}
	else
	{
		SetMovementAnimation();
	}
}


//-----------------------------------------------------------------------------------------------
// Sets the current animation of this actor to one of the idle animations given it's current
// orientation
//
void Actor::SetIdleAnimation()
{
	Vector2 facingDirection = Vector2::MakeDirectionAtDegrees(m_orientationDegrees);

	// Dot products to find general facing direction
	float dotNorth	= DotProduct(facingDirection, Vector2::DIRECTION_UP);
	float dotSouth	= DotProduct(facingDirection, Vector2::DIRECTION_DOWN);
	float dotEast	= DotProduct(facingDirection, Vector2::DIRECTION_RIGHT);
	float dotWest	= DotProduct(facingDirection, Vector2::DIRECTION_LEFT);

	float maxDot	= MaxFloat(dotNorth, dotSouth, dotEast, dotWest);

	// Facing mostly east
	if (maxDot == dotEast)			{ m_spriteAnimSet->StartAnimation("IdleEast"); }

	// Facing mostly west
	else if (maxDot == dotWest)		{ m_spriteAnimSet->StartAnimation("IdleWest"); }

	// Facing mostly north
	else if (maxDot == dotNorth)	{ m_spriteAnimSet->StartAnimation("IdleNorth"); }

	// Facing mostly south
	else							{ m_spriteAnimSet->StartAnimation("IdleSouth"); }
}


//-----------------------------------------------------------------------------------------------
// Sets the current animation of this actor to one of the moving animations given it's current
// velocity - For now the only movement animation is walking
//
void Actor::SetMovementAnimation()
{
	// Dot products to find general movement direction
	float dotNorth = DotProduct(m_velocity, Vector2::DIRECTION_UP);
	float dotSouth = DotProduct(m_velocity, Vector2::DIRECTION_DOWN);
	float dotEast = DotProduct(m_velocity, Vector2::DIRECTION_RIGHT);
	float dotWest = DotProduct(m_velocity, Vector2::DIRECTION_LEFT);

	float maxDot = MaxFloat(dotNorth, dotSouth, dotEast, dotWest);

	// Moving mostly east
	if (maxDot == dotEast)			{ m_spriteAnimSet->StartAnimation("WalkEast"); }

	// Moving mostly west
	else if (maxDot == dotWest)		{ m_spriteAnimSet->StartAnimation("WalkWest"); }

	// Moving mostly north
	else if (maxDot == dotNorth)	{ m_spriteAnimSet->StartAnimation("WalkNorth"); }

	// Moving mostly south
	else							{ m_spriteAnimSet->StartAnimation("WalkSouth"); }
}


//-----------------------------------------------------------------------------------------------
// Updates this entity's equipped items' positions, orientations, and animations to match the actor's
//
void Actor::UpdateEquippedItems(float deltaTime)
{
	for (int i = 0; i < NUM_EQUIP_SLOTS; i++)
	{
		Item* currItem = m_equippedItems[i];
		if (currItem != nullptr)
		{
			currItem->Update(deltaTime);
		}
	}
}
