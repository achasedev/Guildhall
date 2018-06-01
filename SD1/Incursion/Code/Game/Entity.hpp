/************************************************************************/
/* Project: Incursion
/* File: Entity.hpp
/* Author: Andrew Chase
/* Date: September 23rd, 2017
/* Bugs: None
/* Description: Base class for all spawn-able and rendered objects
/************************************************************************/
#pragma once
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/AABB2.hpp"


class Map;

// Enumeration of different sub-classes of entity this entity could be
enum EntityType
{
	ENTITY_TYPE_ERROR = -1,
	ENTITY_TYPE_TURRET,
	ENTITY_TYPE_NPCTANK,
	ENTITY_TYPE_PLAYERTANK,
	ENTITY_TYPE_BULLET,
	ENTITY_TYPE_EXPLOSION,
	ENTITY_TYPE_PICKUP,	
	NUM_ENTITY_TYPES
};

// Faction alliance of this entity (who they attack, are hurt by, etc)
enum Faction
{
	FACTION_ERROR = -1,
	FACTION_PLAYER, 
	FACTION_NEUTRAL,
	FACTION_ENEMY,
	NUM_FACTIONS
};



class Entity
{

public:

	Entity();		// This constructor should NEVER be used explicitly - Entity is an abstract class

	virtual ~Entity();

	//-----Public Methods-----
	virtual void Update(float deltaTime);
	virtual void Render() const;

	void DrawDebugInfo() const;

	//-----Mutators-----
	void SetPosition(const Vector2& newPosition);
	void SetOrientationDegrees(float newOrientationDegress);
	void SetInnerRadius(float newInnerRadius);
	void SetOuterRadius(float newOuterRadius);
	void SetMarkedForDeath(bool markedForDeath);
	void SetAge(float newAge);
	void SetAgeAtDeath(float ageAtDeath);
	void SetFaction(Faction newFaction);
	void SetEntityType(EntityType newType);
	void SetMap(Map* entityMap);
	void SetIsUsingPhysics(bool isUsingPhysics);

	void AddHealth(int healthToAdd);			// Adds health to the entity's current health, clamping at m_maxHealth
	void TakeDamage(int damageAmount);			// Reduces the entity's health by damage amount, clamping at 0

	//-----Accessors-----
	Vector2 GetPosition() const;
	float GetOrientationDegrees() const;
	float GetInnerRadius() const;
	float GetOuterRadius() const;
	bool IsMarkedForDeath() const;
	float GetAge() const;
	float GetAgeAtDeath() const;
	Faction GetFaction() const;
	EntityType GetEntityType() const;
	bool IsUsingPhysics() const;


protected:

	//-----Protected Data-----

	Vector2 m_position;				// Position in 2D space
	float m_orientationDegrees;		// Orientation angle of the object (on unit circle)
	float m_innerRadius;			// Physics radius of the object
	float m_outerRadius;			// Cosmetic radius of the object
	bool m_isMarkedForDeath;		// True if the entity should be destroyed this frame
	int m_maxHealth;				// Total number of hits this Entity can take before being marked for death
	int m_health;					// Current number of hits this entity can take before dying
	bool m_isUsingPhysics;			// Does the entity collide with tiles and other entities?

	float m_age;					// Age in seconds of the object
	float m_ageAtDeath;				// Age of the object when it was marked for destruction

	Faction m_faction;				// Faction alliance of the entity
	EntityType m_entityType;		// Type (subclass) of entity this entity is

	Map* m_map;						// The map this entity was spawned on

	static const float EXPLOSION_RADIUS_MULTIPLIER;			// When the entity dies and leaves an explosion, this is how much larger the explosion is over the entity's cosmetic radius
	static const Vector2 HEALTH_BAR_BOTTOM_LEFT;		// The position of the bottom left corner of the health bar relative to the position of the entity
	static const Vector2 HEALTH_BAR_DIMENSIONS;					// The dimensions of the health bar
};
