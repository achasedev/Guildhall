/************************************************************************/
/* File: Tile.cpp
/* Author: Andrew Chase
/* Date: November 2nd, 2017
/* Bugs: None
/* Description: Implementation of the Tile class
/************************************************************************/
#include "Game/Tile.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/Renderer.hpp"


//-----------------------------------------------------------------------------------------------
// Default constructor, for vector resizing
//
Tile::Tile()
{

}


//-----------------------------------------------------------------------------------------------
// Explicit constructor
//
Tile::Tile(const IntVector2& tileCoords, const TileDefinition* tileDefinition)
	: m_tileCoords(tileCoords)
	, m_tileDef(tileDefinition)
{
}


//-----------------------------------------------------------------------------------------------
// Returns this tile's coordinates within the map
//
IntVector2 Tile::GetTileCoords() const
{
	return m_tileCoords;
}


//-----------------------------------------------------------------------------------------------
// Returns the TileDefinition* this tile currently is
//
const TileDefinition* Tile::GetDefinition() const
{
	return m_tileDef;
}


//-----------------------------------------------------------------------------------------------
// Returns the AABB2 definition this tile's bounds in world coordinates 
//
AABB2 Tile::GetWorldBounds() const
{
	Vector2 bottomLeft = Vector2(GetTileCoords());
	Vector2 topRight = bottomLeft + Vector2::ONES;

	return AABB2(bottomLeft, topRight);
}


//-----------------------------------------------------------------------------------------------
// Returns the base texture UVs used by this tile
//
AABB2 Tile::GetBaseSpriteUVs() const
{
	return m_tileDef->GetBaseSpriteUVs();
}


//-----------------------------------------------------------------------------------------------
// Returns the base color tint used by this tile
//
Rgba Tile::GetBaseTint() const
{
	return m_tileDef->GetBaseTint();
}


//-----------------------------------------------------------------------------------------------
// Returns the overlay texture UVs used by this tile
//
AABB2 Tile::GetOverlaySpriteUVs() const
{
	return m_tileDef->GetOverlaySpriteUVs();
}


//-----------------------------------------------------------------------------------------------
// Returns the overlay color tint used by this tile
//
Rgba Tile::GetOverlayTint() const
{
	return m_tileDef->GetOverlayTint();
}


//-----------------------------------------------------------------------------------------------
// Returns the center position of this tile
//
Vector2 Tile::GetCenterPosition() const
{
	Vector2 bottomLeftCorner = Vector2(GetTileCoords());
	Vector2 halfStep = Vector2(0.5f, 0.5f);
	return bottomLeftCorner + halfStep;
}


//-----------------------------------------------------------------------------------------------
// Returns the texture used by this current tile
//
const Texture& Tile::GetTexture() const
{
	return m_tileDef->GetTileSheetTexture();
}


//-----------------------------------------------------------------------------------------------
// Changes the Tile's set type to the one given
//
void Tile::SetDefinition(const TileDefinition* newDefinition)
{
	m_tileDef = newDefinition;
}


//-----------------------------------------------------------------------------------------------
// Draws the tile to the screen
//
void Tile::Render() const
{
	// Set up coordinate system
	g_theRenderer->PushMatrix();
	g_theRenderer->TranslateCoordinateSystem2D(static_cast<float>(m_tileCoords.x), static_cast<float>(m_tileCoords.y));

	AABB2 texUVs = m_tileDef->GetBaseSpriteUVs();
	Rgba tint = m_tileDef->GetBaseTint();

	// Draws a filled square to screen
	// TODO: Spritesheet - get texture below
	g_theRenderer->DrawTexturedAABB2(AABB2::UNIT_SQUARE_OFFCENTER,  TileDefinition::GetTileSheetTexture(), texUVs, tint);

	// Reset the coordinate system
	g_theRenderer->PopMatrix();
}
