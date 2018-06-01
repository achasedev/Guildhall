/************************************************************************/
/* Project: Incursion
/* File: Map.hpp
/* Author: Andrew Chase
/* Date: September 25th, 2017
/* Bugs: None
/* Description: Class to represent a 2D environment built from 2D tiles
/************************************************************************/
#pragma once
#include <vector>
#include <string>
#include "Game/Tile.hpp"
#include "Game/Entity.hpp"

// Structure to represent Raycast data
struct RaycastResult
{
	bool m_didImpact;					// Did the raycast hit a solid tile?
	Vector2 m_impactPosition;			// Position of impact
	IntVector2 m_impactTileCoords;		// Tile the impact happened on
	float m_impactDistance;				// Distance from start the impact happened
	float m_impactFraction;				// Fractional distance of where the impact happened relative to the max raycast distance
	Vector2 m_impactNormal;				// Surface normal of our impact, currently one of the four cardinal directions (since it's grid based)
};

class Map
{

public:
	//-----Public Methods-----
	Map(int mapNumber);
	~Map();

	void Update(float deltaTime);							// Calls Update() on all entities
	void Render() const;									// Calls Render() on all entities

	void SetupInitialState();								// Creates the tiles and spawns the intial enemies

	Entity* SpawnEntity(Vector2 spawnPosition, float spawnOrientation, Faction entityFaction, EntityType typeToSpawn);
	Entity* SpawnExplosion(Vector2 spawnPosition, float radius, float durationSeconds, bool playSoundEffect); // Spawns an explosion animation, and plays a sound effect if desired
	void PlayRandomExplosionSound() const;					// Plays a random explosion SoundID

	void AddEntityToMap(Entity* entityToAdd);			// Adds the entity to this map's list of entities, for drawing and updating
	void RemovePlayerFromMap();							// Removes the player character from this map
	Entity* FindBestTarget(Entity* hostileEntity);	// Finds the nearest target in line of sight to hostile position and returns it

	Tile GetTileFromPosition(const Vector2& position) const;		// Returns the tile that contains the position 'position'
	Tile GetTileFromCoords(const IntVector2& tileCoords) const;		// Returns the tile at tileCoords (map coordinates)
	
	std::vector<IntVector2> GetRandomSpawnPositions();									// Returns a random spawn location for an entity on a valid tile
	void CorrectEntityEntityCollision(Entity* firstEntity, Entity* secondEntity);		// Moves the two specified collided entities out of eachother evenly

	RaycastResult Raycast(const Vector2& startPosition, const Vector2& direction, float maxDistance) const;		// Performs a step-based raycast from start out maxDistance in direction
	bool HasLineOfSight(const Vector2& startPos, const Vector2& endPos, float viewDistance) const;				// Checks if there is clear line of sight between the two positions and is within range

	bool DidPlayerFinishLevel() const;

	bool IsBeingDestroyed() const;

	void DrawDebugInfo() const;								// Calls DrawDebugInfo() on all entities

private:
	//-----Private Methods-----

	void ProcessMapInput();									// Parses input for updating map state
	void UpdateEntities(float deltaTime);					// Calls update on all entities
	void CheckAllEntityEntityCollisions();					// Performs preventative physics on all entity/entity collisions, barring bullets
	void CheckEntityEntityInteractions();					// Checks if entities have interacted, i.e. bullets hitting hostile enemies
	void DeleteMarkedEntities();							// Deletes all entities marked for delete this frame
	void CheckAllEntityTileCollisions();					// Checks all collisions between entities and tiles

	void CheckBulletAgainstPlayers(Entity* currBullet);
	void CheckBulletAgainstNpcTanks(Entity* currBullet);
	void CheckBulletAgainstNpcTurrets(Entity* currBullet);

	void CheckPickUpPlayerCollisions();															// Checks to see if the player touched any pickups

	TileType GenerateTileType(const IntVector2& tileCoords);									// Randomly assigned interior tiles with either grass or stone types, and exterior ones as stone
	bool IsTileInPlayerStartArea(const IntVector2& tileCoords) const;							// Returns true if the tile coords specify a tile in the player start area
	bool IsTileOnEdgeOfMap(const IntVector2& tileCoords) const;									// Returns true if the tile coords specify a tile that lines the edge of the map
	bool IsTileTheExitTile(const IntVector2& tileCoords) const;									// Returns true if the tile coords specify the level exit tile
	bool IsAdjacentTileSolid(const Tile& currTile, TileCollisionCase currCollisionPoint) const;	// Returns true if the tile adjacent to currTile in the direction case is considered solid

	void CheckEntityForTileCollisions(Entity* entity);										// Checks the given entity for any tiles collisions (8 directions)
	void CheckFixedPointCollision(Entity* entity, Vector2 point);							// Checks to see if the given entity contains the given collision point, and corrects if so
	

	bool IsEntityAlive(Entity* entity) const;			// Checks if the entity is not null and is not marked for death
	void SpawnNpcEntities();							// Spawns a random assortment of npcs to the map
	
	void DrawGridLines() const;							// Draws black grid lines to outline the tiles

private:
	//-----Private Data-----

	// Data structures
	std::vector<Tile> m_tiles;										// Tiles that comprise the map
	std::vector<Entity*> m_entities;								// List of entities on this map, INCLUDES the player character
	std::vector<Entity*> m_entitiesByType[NUM_ENTITY_TYPES];		// List of entities on this map, sorted by type

	// Map parameters
	int m_mapNumber;											// Order this map is in the world, between 0 and 2 inclusive
	int m_numInitialEnemyTanks;
	int m_numIntialEnemyTurrets;
	IntVector2 m_dimensions;										// Dimensions of this map, width (x) by height (y)
	IntVector2 m_levelEndCoords;								// Location of the level exit
	IntVector2 m_playerStartCoords;							// Tile the player spawns on

	bool m_drawGridLines = false;									// Should the grid lines be drawn?
	bool m_isBeingDestroyed = false;								// True when the map is being deleted, so the entities don't spawn explosions or sounds when they die

	static const Vector2 TILE_CENTER_OFFSET;
	static const IntVector2 PLAYER_START_AREA_SIZE;					// Size of the area around the player where enemies won't spawn, and tiles won't be solid
	static const int OUTSIDE_WALL_THICKNESS = 3;					// Number of tiles thick the outside wall is					
	static const int RAYCAST_STEPS_PER_TILE = 100;					// Number of steps used in a raycast per tile


	// Map parameters, for explicitly designing level layout
	static const IntVector2 MAP_ZERO_DIMENSIONS;
	static const IntVector2 MAP_ZERO_PLAYER_START;
	static const IntVector2 MAP_ZERO_LEVEL_END;
	static const int		MAP_ZERO_NUM_TANKS;
	static const int		MAP_ZERO_NUM_TURRETS;

	static const IntVector2 MAP_ONE_DIMENSIONS;
	static const IntVector2 MAP_ONE_PLAYER_START;
	static const IntVector2 MAP_ONE_LEVEL_END;
	static const int		MAP_ONE_NUM_TANKS;
	static const int		MAP_ONE_NUM_TURRETS;

	static const IntVector2 MAP_TWO_DIMENSIONS;
	static const IntVector2 MAP_TWO_PLAYER_START;
	static const IntVector2 MAP_TWO_LEVEL_END;
	static const int		MAP_TWO_NUM_TANKS;
	static const int		MAP_TWO_NUM_TURRETS;
	
	

};