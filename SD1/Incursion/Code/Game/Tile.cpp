/************************************************************************/
/* Project: Incursion
/* File: Tile.cpp
/* Author: Andrew Chase
/* Date: September 25th, 2017
/* Bugs: None
/* Description: Implementation of the Tile class
/************************************************************************/
#include "Game/Tile.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"

//-----------------------------------------------------------------------------------------------
// Constructor - should not be used, constructs an error-type Tile
//
Tile::Tile()
	: m_tileCoords(IntVector2::ZERO)
	, m_tileType(TILE_TYPE_ERROR)
{
}


//-----------------------------------------------------------------------------------------------
// Constructs a tile given it's coordinates in the map and its type
//
Tile::Tile(const IntVector2& tileCoords, TileType tileType)
	: m_tileCoords(tileCoords)
	, m_tileType(tileType)
{
}


//-----------------------------------------------------------------------------------------------
// Draws the tile as a filled AABB2, of solid color
//
void Tile::Render() const
{
	// Set up coordinate system
	g_theRenderer->PushMatrix();
	g_theRenderer->TranslateCoordinateSystem2D(static_cast<float>(m_tileCoords.x), static_cast<float>(m_tileCoords.y));

	TileDefinition& currDefinition = GetTileDefinition();
	AABB2 texCoords = currDefinition.GetTextureCoordinates();
	Rgba tint = currDefinition.GetTint();

	// Draws a filled square to screen
	g_theRenderer->DrawTexturedAABB2(AABB2::UNIT_SQUARE_OFFCENTER, g_tileSpriteSheet->GetTexture(), texCoords.mins, texCoords.maxs, tint);

	// Reset the coordinate system
	g_theRenderer->PopMatrix();
}


//-----------------------------------------------------------------------------------------------
// Returns the coordinates of this tile in the map
//
IntVector2 Tile::GetCoordinates() const
{
	return m_tileCoords;
}


//-----------------------------------------------------------------------------------------------
// Returns this tile's type
//
TileType Tile::GetType() const
{
	return m_tileType;
}


//-----------------------------------------------------------------------------------------------
// Returns the tile definition of this tile, based on its tile type
//
TileDefinition& Tile::GetTileDefinition() const
{
	return TileDefinition::s_tileDefinitions[m_tileType];
}

//-----------------------------------------------------------------------------------------------
// Returns true if the entities can drive over this tile
//
bool Tile::CanDriveOver() const
{
	return GetTileDefinition().CanDriveOver();
}


//-----------------------------------------------------------------------------------------------
// Returns true if the entities can see through this tile
//
bool Tile::CanSeeThrough() const
{
	return GetTileDefinition().CanSeeThrough();
}


//-----------------------------------------------------------------------------------------------
// Returns true if the entities can spawn on this tile
//
bool Tile::CanSpawnOn() const
{
	return GetTileDefinition().CanSpawnOn();
}


//-----------------------------------------------------------------------------------------------
// Returns the collision point associated with this tile, with respect to the location of the
// entity (entity should be ON this tile), and the collision case being calculated.
//
Vector2 Tile::GetCollisionPoint(const Vector2& entityPosition, TileCollisionCase collisionCase) const
{
	Vector2 collisionPoint = entityPosition;
	switch(collisionCase)
	{
	case TILE_COLLISION_CASE_NORTH:
	{
		collisionPoint.y = static_cast<float>(m_tileCoords.y) + 1.f; 
		break;
	}
	case TILE_COLLISION_CASE_SOUTH:
	{
		collisionPoint.y = static_cast<float>(m_tileCoords.y); 
		break;
	}
	case TILE_COLLISION_CASE_EAST:
	{
		collisionPoint.x = static_cast<float>(m_tileCoords.x) + 1.f; 
		break;
	}
	case TILE_COLLISION_CASE_WEST:
	{
		collisionPoint.x = static_cast<float>(m_tileCoords.x); 
		break;
	}
	case TILE_COLLISION_CASE_NORTHEAST:
	{
		collisionPoint.x = static_cast<float>(m_tileCoords.x) + 1.f; 
		collisionPoint.y = static_cast<float>(m_tileCoords.y) + 1.f; 
		break;
	}
	case TILE_COLLISION_CASE_NORTHWEST:
	{
		collisionPoint.x = static_cast<float>(m_tileCoords.x); 
		collisionPoint.y = static_cast<float>(m_tileCoords.y) + 1.f; 
		break;
	}
	case TILE_COLLISION_CASE_SOUTHEAST:
	{
		collisionPoint.x = static_cast<float>(m_tileCoords.x) + 1.f; 
		collisionPoint.y = static_cast<float>(m_tileCoords.y); 
		break;
	}
	case TILE_COLLISION_CASE_SOUTHWEST:
	{
		collisionPoint.x = static_cast<float>(m_tileCoords.x); 
		collisionPoint.y = static_cast<float>(m_tileCoords.y); 
		break;
	}
	default:
		break;
	}
	return collisionPoint;
}
