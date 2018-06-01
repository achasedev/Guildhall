/************************************************************************/
/* File: Map.hpp
/* Author: Andrew Chase
/* Date: November 4th, 2017
/* Bugs: None
/* Description: Class to represent a generated, playable map instance
/************************************************************************/
#pragma once
#include <vector>
#include <string>
#include "Game/Tile.hpp"
#include "Engine/Math/IntVector2.hpp"

class Entity;
class Actor;
class ActorDefinition;
class Projectile;
class ProjectileDefinition;
class Item;
class ItemDefinition;
class Portal;
class PortalDefinition;
class MapDefinition;
class Adventure;


// Structure to represent Raycast data
struct RaycastResult
{
	bool m_didImpact;					// Did the ray cast hit a solid tile?
	Vector2 m_impactPosition;			// Position of impact
	IntVector2 m_impactTileCoords;		// Tile the impact happened on
	float m_impactDistance;				// Distance from start the impact happened
	float m_impactFraction;				// Fractional distance of where the impact happened relative to the max ray cast distance
	Vector2 m_impactNormal;				// Surface normal of our impact, currently one of the four cardinal directions (since it's grid based)
};

class Map
{
public:
	//-----Public Methods-----

	Map(const std::string& name, const MapDefinition* definition, Adventure* adventure);	// Constructs the tiles and applies the generators
	~Map();

	// Tile related
	IntVector2		GetDimensions() const;
	Tile*			GetTileFromPosition(const Vector2& position);
	Tile*			GetTileFromCoords(const IntVector2& tileCoords);
	const Tile*		GetTileFromCoordsConst(const IntVector2& tileCoords) const;
	Tile*			GetTileFromIndex(int index);
	Tile*			GetRandomTileOfType(const TileDefinition* tileType);
	Tile*			GetTileEntityCanEnter(Entity* entity);
	bool			AreCoordsValid(const IntVector2& coords) const;
	bool			EntityCanEnterTile(Entity* entity, const Tile* tile);

	void Update(float delteTime);
	void Render() const;

	// Entity related
	void SpawnActor(const Vector2& position, float orientation, const ActorDefinition* definition, const std::string& name);
	Projectile* SpawnProjectile(const Vector2& position, float orientation, const ProjectileDefinition* definition, const std::string& name, const std::string& faction);
	void SpawnPortal(const Vector2& position, float orientation, const PortalDefinition* definition, const std::string& name, Map* destinationMap, const Vector2& destinationPosition);
	void SpawnItem(const Vector2& position, float orientation, const ItemDefinition* definition, const std::string& name);

	void RemoveActorFromMap(Actor* actorToRemove);
	void RemoveProjectileFromMap(Projectile* projectileToRemove);
	void RemoveItemFromMap(Item* itemToRemove);

	void AddActorToMap(Actor* actorToAdd);
	void AddItemToMap(Item* itemToAdd);
	void AddProjectileToMap(Projectile* projectileToAdd);
	void AddPortalToMap(Portal* portalToAdd);

	void DisableAllPortals();

	Actor*	FindBestHostileActor(Actor* attackingActor) const;
	bool	AreSameFaction(Entity* a, Entity* b) const;

	// Ray casting
	RaycastResult			Raycast(const Vector2& startPosition, const Vector2& direction, float maxDistance) const;		// Performs a step-based raycast from start out maxDistance in direction
	bool					HasLineOfSight(const Vector2& startPos, const Vector2& endPos, float viewDistance) const;		// Checks if there is clear line of sight between the two positions and is within range

	bool IsBeingDeleted() const;

private:
	//-----Private Methods-----

	// Main Update Loop
	void ProcessMapInput();
	void UpdateEntities(float deltaTime);
	void CheckForAndCorrectActorCollisions();
	void CheckForAndCorrectTileCollisions();
	void CheckProjectilesAgainstActors();
	void CheckActorsAgainstPortals();
	void CheckForItemPickup();
	void DeleteMarkedEntities();

	// Render Functions
	void RenderTiles() const;
	void DrawPickupText() const;


	// Physics Functions
	void CheckEntityForAdjacentTileCollisions(Entity* entity, bool shouldDestroy);
	void CheckEntityTileCollision(Entity* entity, const IntVector2& adjTileCoords, const Vector2& collisionPoint, bool shouldDestroy);

	void CorrectActorCollision(Actor* firstActor, Actor* secondActor);


	// Entity Functions
	void SortEntitiesByPosition();
	bool IsEntityAlive(Entity* entity) const;


private:
	//-----Private Data-----

	std::string				m_name;				// The name of this map instance
	IntVector2				m_dimensions;		// The width/height of this map
	std::vector<Tile>		m_tiles;			// Tiles of this map, indexed left to right and then up the map
	const MapDefinition*	m_definition;		// The type of map this map is
	Adventure*				m_theAdventure;		// The adventure this map belongs to					

	Item* m_itemPlayerIsOn;						// Reference to the item the player is standing on, nullptr if no item
	bool m_isBeingDeleted = false;

	// Entity lists
	std::vector<Entity*>		m_entities;		// All entities being rendered and updated on this map
	std::vector<Actor*>			m_actors;		// Actors on this map
	std::vector<Projectile*>	m_projectiles;	// Projectiles on this map
	std::vector<Portal*>		m_portals;		// Portals on this map
	std::vector<Item*>			m_items;		// Items on this map, not in actor inventories or equipped

	// Static Data
	static const int RAYCAST_STEPS_PER_TILE = 100;		// Number of steps used in a ray cast per tile

	static const AABB2 PICKUP_DRAW_BOX;
};
