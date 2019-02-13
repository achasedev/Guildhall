/************************************************************************/
/* File: EntitySpawnEvent_OffScreen.cpp
/* Author: Andrew Chase
/* Date: February 12th 2019
/* Description: Implementation of the OffScreen spawn event
/************************************************************************/
#include "Game/Entity/AIEntity.hpp"
#include "Game/Entity/EntitySpawnEvent_OffScreen.hpp"
#include "Engine/Math/MathUtils.hpp"


//---C FUNCTION----------------------------------------------------------------------------------
// Returns an AABB2 representing the spawn region that would cause enemies to come from the given
// direction on the map (i.e. "north" returns an area where enemies will spawn from the north)
//
AABB2 GetBoundsForSpawnDirection(const std::string& directionText)
{
	if		(directionText == "south")		{ return AABB2(Vector2(0.f, -16.f), Vector2(256.f, 0.f)); }
	else if (directionText == "east")		{ return AABB2(Vector2(256.f, 0.f), Vector2(272.f, 256.f)); }
	else if (directionText == "west")		{ return AABB2(Vector2(-16.f, 0.f), Vector2(0.f, 256.f)); }
	else if (directionText == "northeast")	{ return AABB2(Vector2(256.f), Vector2(272.f)); }
	else if (directionText == "northwest")	{ return AABB2(Vector2(-16.f, 256.f), Vector2(0.f, 272.f)); }
	else if (directionText == "southeast")	{ return AABB2(Vector2(256.f, -16.f), Vector2(272.f, 0.f)); }
	else if (directionText == "southwest")	{ return AABB2(Vector2(-16.f, -16.f), Vector2::ZERO); }
	else // North Case
	{
		return AABB2(Vector2(0.f, 256.f), Vector2(256.f, 272.f)); 
	}
}


//-----------------------------------------------------------------------------------------------
// Constructor
//
EntitySpawnEvent_OffScreen::EntitySpawnEvent_OffScreen(const XMLElement& element)
	: EntitySpawnEvent(element)
{
	m_spawnDirection = ParseXmlAttribute(element, "spawn_direction", "north");
	m_areaToSpawnIn = GetBoundsForSpawnDirection(m_spawnDirection);
}


//-----------------------------------------------------------------------------------------------
// Update - not used
//
void EntitySpawnEvent_OffScreen::Update()
{
}


//-----------------------------------------------------------------------------------------------
// Render
//
int EntitySpawnEvent_OffScreen::RunSpawn()
{
	int countToSpawn = MinInt(m_spawnRate, GetEntityCountLeftToSpawn());

	for (int spawnNumber = 0; spawnNumber < countToSpawn; ++spawnNumber)
	{
		Vector2 xzPos = m_areaToSpawnIn.GetRandomPointInside();
		Vector3 position = Vector3(xzPos.x, 1.f, xzPos.y);

		AIEntity* entity = SpawnEntity(position, 0.f);
		entity->SetShouldCheckForEdgeCollisions(false);
	}

	return countToSpawn;
}


//-----------------------------------------------------------------------------------------------
// For cloning this entity spawn event for prototyping
//
EntitySpawnEvent* EntitySpawnEvent_OffScreen::Clone(CampaignManager* manager) const
{
	EntitySpawnEvent_OffScreen* offScreenEvent = new EntitySpawnEvent_OffScreen(*this);
	offScreenEvent->m_manager = manager;

	return offScreenEvent;
}
