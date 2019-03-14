/************************************************************************/
/* File: EntityDefinition.hpp
/* Author: Andrew Chase
/* Date: October 2nd, 2018
/* Description: Class to represent the data values behind an entity
/************************************************************************/
#pragma once
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/IntVector3.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/Utility/XmlUtilities.hpp"

#include <map>
#include <string>
#include <vector>

// For determining what Entity subclass to use for this definition when constructing
enum eEntityClass
{
	ENTITY_CLASS_UNASSIGNED,
	ENTITY_CLASS_STATIC,
	ENTITY_CLASS_AI,
	ENTITY_CLASS_WEAPON,
	ENTITY_CLASS_PROJECTILE,
	ENTITY_CLASS_ITEM,

	ENTITY_CLASS_PLAYER,
	ENTITY_CLASS_PARTICLE,
	ENTITY_CLASS_CHARACTERSELECTVOLUME
};

// Convenience type to know what kind of entity we are
enum ePhysicsType
{
	PHYSICS_TYPE_STATIC,
	PHYSICS_TYPE_DYNAMIC,
	PHYSICS_TYPE_UNASSIGNED
};


// How the entity should react to collision corrections
enum eCorrectionResponse
{
	COLLISION_RESPONSE_IGNORE_CORRECTION,
	COLLISION_RESPONSE_NO_CORRECTION,
	COLLISION_RESPONSE_SHARE_CORRECTION,
	COLLISION_RESPONSE_FULL_CORRECTION,
	NUM_COLLISION_RESPONSES
};


// Which bits correspond to which layer (columns of collision matrix)
enum eCollisionLayerBit : uint8_t
{
	COLLISION_LAYER_BIT_WORLD = (1 << 0),
	COLLISION_LAYER_BIT_PLAYER = (1 << 1),
	COLLISION_LAYER_BIT_ENEMY = (1 << 2),
	COLLISION_LAYER_BIT_PLAYER_BULLET = (1 << 3),
	COLLISION_LAYER_BIT_ENEMY_BULLET = (1 << 4),
	COLLISION_LAYER_BIT_ITEM = (1 << 5)
};


// Rows of collision matrix
enum eCollisionLayer : uint8_t
{
	COLLISION_LAYER_WORLD = (0xff), // Collide with everything
	COLLISION_LAYER_PLAYER = (COLLISION_LAYER_BIT_WORLD | COLLISION_LAYER_BIT_PLAYER | COLLISION_LAYER_BIT_ENEMY | COLLISION_LAYER_BIT_ENEMY_BULLET),
	COLLISION_LAYER_ENEMY = (COLLISION_LAYER_BIT_WORLD | COLLISION_LAYER_BIT_PLAYER | COLLISION_LAYER_BIT_ENEMY | COLLISION_LAYER_BIT_PLAYER_BULLET),
	COLLISION_LAYER_PLAYER_BULLET = (COLLISION_LAYER_BIT_WORLD | COLLISION_LAYER_BIT_ENEMY),
	COLLISION_LAYER_ENEMY_BULLET = (COLLISION_LAYER_BIT_WORLD | COLLISION_LAYER_BIT_PLAYER),
	COLLISION_LAYER_ITEM = (COLLISION_LAYER_BIT_WORLD | COLLISION_LAYER_BIT_PLAYER)
};

enum eWeaponSpreadType
{
	SPREAD_NONE,
	SPREAD_RANDOM,
	SPREAD_FAN,
	SPREAD_SOURCE
};


// Collision state for a single entity
struct CollisionDefinition_t
{
	CollisionDefinition_t() {}
	CollisionDefinition_t(eCorrectionResponse response, eCollisionLayer collisionLayer, bool canDestroyVoxels, float collisionKnockback)
		: m_response(response), layer(collisionLayer), m_canDestroyVoxels(canDestroyVoxels), m_collisionKnockback(collisionKnockback) {}

	eCollisionLayer				layer = COLLISION_LAYER_WORLD;
	eCorrectionResponse			m_response = COLLISION_RESPONSE_SHARE_CORRECTION;
	bool						m_canDestroyVoxels = false;
	float						m_collisionKnockback = 0.f;
};


class VoxelSprite;
class VoxelAnimationSet;
class BehaviorComponent;

class EntityDefinition
{
public:
	//-----Public Methods-----

	EntityDefinition() {} // Only to be used by Campaign manager for character select!
	EntityDefinition(const XMLElement& entityElement, eEntityClass entityClass);

	BehaviorComponent* ConstructBehaviorPrototype(const XMLElement& behaviorElement);
	
	std::string			GetName() const;
	IntVector3			GetDimensions() const;
	BehaviorComponent*	CloneBehaviorPrototype() const;

	bool				HasGravity() const;
	int					GetInitialHealth() const;
	const VoxelSprite*	GetDefaultSprite() const;

	static void						LoadDefinitions(const std::string& filename);
	static const EntityDefinition*	GetDefinition(const std::string& defName);
	static const EntityDefinition*	GetRandomPlayerDefinition();
	static void						AddDefinition(const EntityDefinition* definition);
	static void						GetAllCharacterSelectVolumeDefinitions(std::vector<const EntityDefinition*>& out_definitions);


public:
	//-----Private Data-----
	
	// Entity Base class
	std::string								m_name;
	int										m_initialHealth = 99999;
	ePhysicsType							m_physicsType = PHYSICS_TYPE_STATIC;
	bool									m_affectedByGravity = true;
	CollisionDefinition_t					m_collisionDef;
	eEntityClass							m_entityClass = ENTITY_CLASS_UNASSIGNED;

	SoundID									m_onSpawnSound = MISSING_SOUND_ID;
	SoundID									m_onDeathSound = MISSING_SOUND_ID;
	SoundID									m_onDamageTakenSound = MISSING_SOUND_ID;
	SoundID									m_customSound = MISSING_SOUND_ID;

	// AnimatedEntity
	float									m_maxMoveAcceleration = 300.f;
	float									m_maxMoveSpeed = 40.f;
	float									m_maxMoveDeceleration = 100.f;
	float									m_jumpImpulse = 80.f;

	const VoxelAnimationSet*				m_animationSet = nullptr;
	const VoxelSprite*						m_defaultSprite = nullptr;
	bool									m_isDestructible = false;
	bool									m_isAnimated = false;

	// AIEntity/Enemies
	const BehaviorComponent*				m_behaviorPrototype;
	int										m_pointValue = 100;
	std::string								m_lootTableName;

	// Weapons
	const EntityDefinition*					m_projectileDefinition = nullptr;
	float									m_fireRate = 1.0f;
	float									m_fireSpread = 0.f;
	int										m_projectilesFiredPerShot = 1;
	int										m_initialAmmoCount = 1;
	eWeaponSpreadType						m_spreadType = SPREAD_NONE;
	bool									m_isFullAuto = true;

	SoundID									m_onPickupSound = MISSING_SOUND_ID;
	SoundID									m_onShootSound = MISSING_SOUND_ID;

	// Projectiles
	float									m_projectileSpeed = 100.f;
	float									m_projectileLifetime = 2.f;
	float									m_projectileHitRadius = 0.f;
	int										m_projectileDamage = 1;

	// Character Select Volume
	const EntityDefinition*					m_playerCharacterDefinition = nullptr;
	
	// Static registry
	static std::map<std::string, const EntityDefinition*> s_definitions;

};
