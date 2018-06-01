/************************************************************************/
/* File: Entity.hpp
/* Author: Andrew Chase
/* Date: November 14th, 2017
/* Bugs: None
/* Description: Base class for all spawn-able and rendered objects
/************************************************************************/
#pragma once
#include "Game/EntityDefinition.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/AABB2.hpp"
#include <string>

class Map;
class SpriteAnim;
class SpriteAnimSet;


class Entity
{

public:
	//-----Public Methods-----

	Entity(const Vector2& position, float orientation, const EntityDefinition* definition, Map* entityMap, const std::string& name);
	~Entity();

	virtual void Update(float deltaTime);
	virtual void Render() const;

	//-----Mutators-----
	void	SetPosition(const Vector2& newPosition);
	void	SetVelocity(const Vector2& newVelocity);
	void	AddToVelocity(const Vector2& addAmount);
	void	SetOrientationDegrees(float newOrientationDegress);
	void	SetAngularVelocity(float newAngularVelocity);
	void	SetMarkedForDeath(bool newState);
	void	SetIsUsingPhysics(bool newState);
	void	ToggleIsUsingPhysics();
	void	SetMap(Map* newMap);
	void	StartAnimation(const std::string& animName);
	void	PauseAnimation();
	void	PlayAnimation();
	void	SetStats(const Stats& newStats);

	void	DoDamageCheck(Entity* attackingEntity);

	//-----Accessors-----
	std::string				GetName() const;
	virtual std::string		GetFaction() const;
	Vector2					GetPosition() const;
	Vector2					GetVelocity() const;
	float					GetOrientationDegrees() const;
	float					GetAngularVelocity() const;
	float					GetInnerRadius() const;
	AABB2					GetLocalDrawBounds() const;
	Map*					GetMap() const;
	bool					GetIsUsingPhysics() const;
	int						GetCurrentHealth() const;
	int						GetMaxHealth() const;
	virtual Stats			GetStats() const;
	bool					CanSee() const;
	bool					CanWalk() const;
	bool					CanFly() const;
	bool					CanSwim() const;
	float					GetViewDistance() const;
	float					GetWalkSpeed() const;
	float					GetFlySpeed() const;
	float					GetSwimSpeed() const;
	SpriteAnim*				GetCurrentAnimation() const;
	std::string				GetCurrentAnimationName() const;
	SpriteAnimSet*			GetSpriteAnimSet() const;
	bool					IsMarkedForDeletion() const;
	bool					IsUsingPhysics() const;

	// Producers
	AABB2					GetWorldDrawBounds() const;

private:

	void InitializeBaseStats(const EntityDefinition* entityDefinition);

protected:
	//-----Protected Data-----

	std::string				m_name;						// Name of the entity

	// Positional
	Vector2					m_position;					// Position in 2D space
	Vector2					m_velocity;					// Velocity in OpenGL-world units
	Vector2					m_force;					// Current force this Entity is undergoing
	float					m_orientationDegrees;		// Orientation angle of the object (on unit circle)
	float					m_angularVelocity;			// Rotation velocity of the object

	// Health/Stats
	int						m_health;					// Health of this entity, zero means death
	float					m_age;						// Age of the entity, incremented every frame by deltaSeconds
	float					m_ageAtDeath;				// Age of the entity at destruction
	Stats					m_baseStats;				// This entity's base stats

	// Meta
	bool					m_isMarkedForDeletion;		// True if the entity is to be deleted this frame
	bool					m_isUsingPhysics;			// False if this entity should be skipped in the physics step

	// Graphics
	SpriteAnimSet* m_spriteAnimSet;						// The collection of animations this entity uses

	// Static
	const EntityDefinition* m_entityDefinition;			// Static data corresponding with this entity
	Map*					m_map;						// The map this entity is on
};
