/************************************************************************/
/* File: Item.cpp
/* Author: Andrew Chase
/* Date: November 28th, 2017
/* Bugs: None
/* Description: Implementation of the Item class
/************************************************************************/
#include "Game/Item.hpp"
#include "Game/Actor.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/SpriteAnim.hpp"
#include "Engine/Renderer/SpriteAnimSet.hpp"
#include "Engine/Renderer/Renderer.hpp"

//-----------------------------------------------------------------------------------------------
// Constructor - Constructs given entity information, as well as a definition to construct from
//
Item::Item(const Vector2& position, float orientation, const ItemDefinition* definition, Map* entityMap, const std::string& name, Actor* actor/*=nullptr*/)
	: Entity(position, orientation, definition, entityMap, name)
	, m_itemDefinition(definition)
	, m_actorHoldingItem(actor)
{
}


//-----------------------------------------------------------------------------------------------
// Sets the current animation to the same as the parent actor, and calls the base Entity::Update()
//
void Item::Update(float deltaTime)
{
	if (m_actorHoldingItem != nullptr)
	{
		UpdateAnimationFromActor(deltaTime);
	}

	deltaTime = 0.f;
}


//-----------------------------------------------------------------------------------------------
// Draws the item based on the animation state in the Entity base class
//
void Item::Render() const
{
	if (m_actorHoldingItem != nullptr)
	{
		RenderAnimationFromActor();
	}
	else
	{
		Entity::Render();	// Render using its own position
	}
	
}


//-----------------------------------------------------------------------------------------------
// Sets the owner of this item to the one specified (for equipping)
//
void Item::SetActorHoldingThis(Actor* actorToHoldThisItem)
{
	m_actorHoldingItem = actorToHoldThisItem;
}


//-----------------------------------------------------------------------------------------------
// Returns the equip slot of the item
//
EquipSlot Item::GetEquipSlot() const
{
	return m_itemDefinition->GetEquipSlot();
}


//-----------------------------------------------------------------------------------------------
// Updates the animation of this item from the actor's animation
//
void Item::UpdateAnimationFromActor(float deltaTime)
{
	// Call update on the animation just to check for isFinished state
	if (m_spriteAnimSet->GetCurrentAnimation() != nullptr)
	{
		m_spriteAnimSet->Update(deltaTime);
	}

	std::string currActorAnimName = m_actorHoldingItem->GetCurrentAnimationName();
	float actorAnimTimeElapsed = m_actorHoldingItem->GetCurrentAnimation()->GetTotalSecondsElapsed();

	// Set the animation values to match
	m_spriteAnimSet->StartAnimation(currActorAnimName);
	m_spriteAnimSet->SetCurrentTimeElapsed(actorAnimTimeElapsed);
}


//-----------------------------------------------------------------------------------------------
// Renders the animation on the actor's position
//
void Item::RenderAnimationFromActor() const
{
	// Draw the animation set's current animation
	const Texture& texture	= m_spriteAnimSet->GetTexture();
	AABB2 currentUVs		= m_spriteAnimSet->GetCurrentUVs();
	bool shouldOrient		= m_spriteAnimSet->GetCurrentAnimation()->ShouldAutoOrient();

	// Set up the coordinate system
	g_theRenderer->PushMatrix();
	g_theRenderer->TranslateCoordinateSystem2D(m_actorHoldingItem->GetPosition());

	if (shouldOrient)
	{
		g_theRenderer->RotateCoordinateSystem2D(m_actorHoldingItem->GetOrientationDegrees());
	}

	g_theRenderer->DrawTexturedAABB2(m_entityDefinition->GetLocalDrawBounds(), texture, currentUVs, Rgba::WHITE);

	g_theRenderer->PopMatrix();
}
