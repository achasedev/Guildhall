/************************************************************************/
/* File: Tile.hpp
/* Author: Andrew Chase
/* Date: November 2nd, 2017
/* Bugs: None
/* Description: Class to represent a physical tile in a map
/************************************************************************/
#pragma once
#include "Engine/Math/IntVector2.hpp"
#include "Game/TileDefinition.hpp"


class Tile
{
public:
	//-----Public Methods-----
	Tile();
	Tile(const IntVector2& tileCoords, const TileDefinition* tileDefinition);

	IntVector2				GetTileCoords() const;
	const TileDefinition*	GetDefinition() const;
	AABB2					GetWorldBounds() const;
	AABB2					GetBaseSpriteUVs() const;
	Rgba					GetBaseTint() const;
	AABB2					GetOverlaySpriteUVs() const;
	Rgba					GetOverlayTint() const;
	Vector2					GetCenterPosition() const;
	const Texture&			GetTexture() const;
	void					SetDefinition(const TileDefinition* newDefinition);

	void Render() const;

private:
	//-----Private Data-----

	IntVector2 m_tileCoords;					// Position of this tile in the map
	const TileDefinition* m_tileDef = nullptr;		// The type of tile this tile is
};