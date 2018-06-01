/************************************************************************/
/* Project: Incursion
/* File: TileDefinition.hpp
/* Author: Andrew Chase
/* Date: October 3rd, 2017
/* Bugs: None
/* Description: Class to represent a visual/physical definition of a tile
/************************************************************************/
#pragma once
#include "Engine/Math/AABB2.hpp"
#include "Engine/Core/Rgba.hpp"
#include "Engine/Math/IntVector2.hpp"


// Enumeration for the various types of tiles
enum TileType {
	TILE_TYPE_ERROR = -1, 
	TILE_TYPE_GRASS,
	TILE_TYPE_STONE,
	TILE_TYPE_WATER,
	TILE_TYPE_PLAYER_START,
	TILE_TYPE_END_LEVEL,
	NUM_TILE_TYPES
};


class TileDefinition
{
public:
	//-----Public Methods-----

	TileDefinition();		// Constructs a tile of error values
	TileDefinition(bool canDriveOver, bool canSeeThrough, bool canShootThrough, bool canSpawnOn, const IntVector2& spriteCoords, const Rgba& tint);		// Explicit constructor

	static void InitializeTileDefinitions();	// Initializes the TileDefinition list, to define each type of tile

	// Accessors for private members
	bool CanDriveOver() const;
	bool CanSeeThrough() const;
	bool CanSpawnOn() const;

	AABB2 GetTextureCoordinates() const;		
	Rgba GetTint() const;						

public:
	//-----Public Data-----

	static TileDefinition s_tileDefinitions[NUM_TILE_TYPES];	// The list of definitions used in this game


private:
	//-----Private Data-----

	bool m_canDriveOver;		// Do entities collide with this tile?
	bool m_canSeeThrough;		// Can raycasts pass through this tile?
	bool m_canShootThrough;		// Can bullets pass through this tile?
	bool m_canSpawnOn;			// Can enemies spawn on this tile?
	AABB2 m_texCoords;			// The texture coordinates of this definition in the tile sprite map
	Rgba m_tint;				// The color tint to be applied to this definition's texture when drawing
};
