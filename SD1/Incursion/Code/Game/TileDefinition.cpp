/************************************************************************/
/* Project: Incursion
/* File: TileDefinition.cpp
/* Author: Andrew Chase
/* Date: October 3rd, 2017
/* Bugs: None
/* Description: Implementation of the TileDefinition class
/************************************************************************/
#include "Game/GameCommon.hpp"
#include "Game/TileDefinition.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"

// Define static member
TileDefinition TileDefinition::s_tileDefinitions[NUM_TILE_TYPES];


//-----------------------------------------------------------------------------------------------
// Default constructor - defines an error TileDefinition
//
TileDefinition::TileDefinition()
	: m_canDriveOver(false)
	, m_canSeeThrough(false)
	, m_canShootThrough(false)
	, m_canSpawnOn(false)
	, m_texCoords(AABB2(Vector2(0.f, 1.f), Vector2(1.f, 0.f)))
	, m_tint(Rgba())
{
}


//-----------------------------------------------------------------------------------------------
// Constructor - creates a definition given whether or not it's solid, the sprite coordinates
// in the terrain sprite sheet, and the tint used to draw
//
TileDefinition::TileDefinition(bool canDriveOver, bool canSeeThrough, bool canShootThrough, bool canSpawnOn, const IntVector2& spriteCoords, const Rgba& tint)
	: m_canDriveOver(canDriveOver)
	, m_canSeeThrough(canSeeThrough)
	, m_canShootThrough(canShootThrough)
	, m_canSpawnOn(canSpawnOn)
	, m_texCoords(g_tileSpriteSheet->GetTexCoordFromSpriteCoords(spriteCoords))
	, m_tint(tint)
{
}


//-----------------------------------------------------------------------------------------------
// Sets up the tile definitions array so that tiles may begin referencing them
//
void TileDefinition::InitializeTileDefinitions()
{
	s_tileDefinitions[TILE_TYPE_GRASS] = TileDefinition(true, true, true, true, IntVector2(1, 1), Rgba::GREEN);
	s_tileDefinitions[TILE_TYPE_STONE] = TileDefinition(false, false, false, false, IntVector2(0, 2), Rgba::WHITE);
	s_tileDefinitions[TILE_TYPE_WATER] = TileDefinition(false, true, true, false, IntVector2(5, 7), Rgba::LIGHT_BLUE);
	s_tileDefinitions[TILE_TYPE_PLAYER_START] = TileDefinition(true, true, true, false, IntVector2(5, 4), Rgba::WHITE);
	s_tileDefinitions[TILE_TYPE_END_LEVEL] = TileDefinition(true, true, true, false, IntVector2(1, 7), Rgba::WHITE);
}


//-----------------------------------------------------------------------------------------------
// Returns true if the entities can drive over this tile
//
bool TileDefinition::CanDriveOver() const
{
	return m_canDriveOver;
}


//-----------------------------------------------------------------------------------------------
// Returns true if the entities can see through this tile
//
bool TileDefinition::CanSeeThrough() const
{
	return m_canSeeThrough;
}


//-----------------------------------------------------------------------------------------------
// Returns true if the entities can spawn on this tile
//
bool TileDefinition::CanSpawnOn() const
{
	return m_canSpawnOn;
}


//-----------------------------------------------------------------------------------------------
// Returns the texture coordinates from the sprite sheet for this tile definition
//
AABB2 TileDefinition::GetTextureCoordinates() const
{
	return m_texCoords;
}


//-----------------------------------------------------------------------------------------------
// Returns the tint associated with this tile definition's texture
//
Rgba TileDefinition::GetTint() const
{
	return m_tint;
}
