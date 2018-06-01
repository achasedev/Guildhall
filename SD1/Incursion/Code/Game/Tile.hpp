/************************************************************************/
/* Project: Incursion
/* File: Tile.hpp
/* Author: Andrew Chase
/* Date: September 25th, 2017
/* Bugs: None
/* Description: Class to represent squares on a map
/************************************************************************/
#pragma once
#include "Game/TileDefinition.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/IntVector2.hpp"


// For Entity/Tile collisions, when determining the location of the collision point
enum TileCollisionCase
{
	TILE_COLLISION_CASE_ERROR = -1,
	TILE_COLLISION_CASE_NORTH,
	TILE_COLLISION_CASE_SOUTH,
	TILE_COLLISION_CASE_EAST,
	TILE_COLLISION_CASE_WEST,
	TILE_COLLISION_CASE_NORTHEAST,
	TILE_COLLISION_CASE_NORTHWEST,
	TILE_COLLISION_CASE_SOUTHEAST,
	TILE_COLLISION_CASE_SOUTHWEST,
	NUM_TILE_COLLISION_CASES
};


class Tile
{

public:
	
	Tile();														// Shouldn't be used, constructs an error-type Tile
	Tile(const IntVector2& tileCoords, TileType tileType);		// Constructs a tile explicitly

	void Render() const;										// Draws the tile to the screen, as an AABB2

	//-----Accessors-----
	IntVector2 GetCoordinates() const;							
	TileType GetType() const;
	TileDefinition& GetTileDefinition() const;

	bool CanDriveOver() const;
	bool CanSeeThrough() const;
	bool CanSpawnOn() const;

	// Returns a collision point of this tile, given the entity's position and the case of collision
	Vector2 GetCollisionPoint(const Vector2& entityPosition, TileCollisionCase collisionCase) const;

public:

private:

private:
	//-----Private Data-----

	IntVector2 m_tileCoords;			// Zero-based (x,y) position in map dimensions
	TileType m_tileType;				// Type of tile (stone, grass, etc)

};