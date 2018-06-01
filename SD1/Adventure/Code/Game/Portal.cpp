/************************************************************************/
/* File: Portal.cpp
/* Author: Andrew Chase
/* Date: November 19th, 2017
/* Bugs: None
/* Description: Implementation of the Portal class
/************************************************************************/
#include "Game/Portal.hpp"
#include "Game/PortalDefinition.hpp"
#include "Game/TileDefinition.hpp"
#include "Game/Map.hpp"
#include "Game/Actor.hpp"
#include "Engine/Renderer/SpriteAnimSet.hpp"


//-----------------------------------------------------------------------------------------------
// Constructs an portal given the portal definition and entity information
//
Portal::Portal(const Vector2& position, float orientation, const PortalDefinition* definition, Map* spawnMap, const std::string& name, Map* destinationMap, const Vector2& destinationPosition)
	: Entity(position, orientation, definition, spawnMap, name)
	, m_destinationMap(destinationMap)
	, m_destinationPosition(destinationPosition)
{
	m_spriteAnimSet->StartAnimation("WorldSprite");
}


//-----------------------------------------------------------------------------------------------
// Updates this portal each frame
//
void Portal::Update(float deltaTime)
{
	Entity::Update(deltaTime);
}


//-----------------------------------------------------------------------------------------------
// Draws the portal to screen 
//
void Portal::Render() const
{
	Entity::Render();
}


//-----------------------------------------------------------------------------------------------
// Removes the actor from its current map and moves it to this portal's destination map and position
//
void Portal::MoveActorToDestinationMap(Actor* actorToMove)
{
	m_map->RemoveActorFromMap(actorToMove);
	m_destinationMap->AddActorToMap(actorToMove);
	actorToMove->SetPosition(m_destinationPosition + Vector2::GetRandomVector(0.05f)); // Slight offset to prevent two entities from being on top of each other exactly
	actorToMove->SetMap(m_destinationMap);
}


//-----------------------------------------------------------------------------------------------
// Returns true if this portal is ready to move actors to it's destination map/position
//
bool Portal::IsReady() const
{
	return m_isReady;
}


//-----------------------------------------------------------------------------------------------
// Returns this portal's destination map
//
Map* Portal::GetDestinationMap() const
{
	return m_destinationMap;
}


//-----------------------------------------------------------------------------------------------
// Sets this portal's ready flag to newState
//
void Portal::SetIsReady(bool newState)
{
	m_isReady = newState;
}
