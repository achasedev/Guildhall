/************************************************************************/
/* File: EntityDefinition.hpp
/* Author: Andrew Chase
/* Date: October 2nd, 2018
/* Description: Class to represent the data values behind an entity
/************************************************************************/
#pragma once
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/IntVector3.hpp"
#include "Engine/Core/Utility/XmlUtilities.hpp"

#include <map>
#include <string>
#include <vector>

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
	COLLISION_LAYER_BIT_ENEMY_BULLET = (1 << 4)
};


// Rows of collision matrix
enum eCollisionLayer : uint8_t
{
	COLLISION_LAYER_WORLD = (0xff), // Collide with everything
	COLLISION_LAYER_PLAYER = (COLLISION_LAYER_BIT_WORLD | COLLISION_LAYER_BIT_PLAYER | COLLISION_LAYER_BIT_ENEMY | COLLISION_LAYER_BIT_ENEMY_BULLET),
	COLLISION_LAYER_ENEMY = (COLLISION_LAYER_BIT_WORLD | COLLISION_LAYER_BIT_PLAYER | COLLISION_LAYER_BIT_ENEMY | COLLISION_LAYER_BIT_PLAYER_BULLET),
	COLLISION_LAYER_PLAYER_BULLET = (COLLISION_LAYER_BIT_WORLD | COLLISION_LAYER_BIT_ENEMY),
	COLLISION_LAYER_ENEMY_BULLET = (COLLISION_LAYER_BIT_WORLD | COLLISION_LAYER_BIT_PLAYER)
};


// Collision state for a single entity
struct CollisionDefinition_t
{
	CollisionDefinition_t() {}
	CollisionDefinition_t(eCorrectionResponse response, eCollisionLayer collisionLayer)
		: m_response(response), layer(collisionLayer) {}

	eCollisionLayer				layer = COLLISION_LAYER_WORLD;
	eCorrectionResponse			m_response = COLLISION_RESPONSE_SHARE_CORRECTION;
};

// For items
struct ItemSet_t
{
	void operator+=(const ItemSet_t& toAdd)
	{
		bullets += toAdd.bullets;
		shells += toAdd.shells;
		energy += toAdd.energy;
		explosives += toAdd.explosives;
		money += toAdd.money;
	}

	const ItemSet_t& operator+(const ItemSet_t& toAdd) const
	{
		ItemSet_t toReturn;

		toReturn.bullets += toAdd.bullets;
		toReturn.shells += toAdd.shells;
		toReturn.energy += toAdd.energy;
		toReturn.explosives += toAdd.explosives;
		toReturn.money += toAdd.money;

		return toReturn;
	}

	int bullets = 0;
	int shells = 0;
	int energy = 0;
	int explosives = 0;
	int money = 0;
};

class VoxelSprite;
class VoxelAnimationSet;
class BehaviorComponent;

class EntityDefinition
{
	friend class Entity;
	friend class AnimatedEntity;
	friend class Player;
	friend class Item;

public:
	//-----Public Methods-----
	
	
	std::string	GetName() const;
	IntVector3 GetDimensions() const;
	BehaviorComponent* CloneBehaviorPrototype(unsigned int index) const;

	bool	HasGravity() const;
	int		GetDefaultHealth() const;

	static void						LoadDefinitions(const std::string& filename);
	static const EntityDefinition*	GetDefinition(const std::string& defName);


private:
	//-----Private Methods-----
	
	EntityDefinition(const XMLElement& entityElement);

	BehaviorComponent* ConstructBehaviorPrototype(const XMLElement& behaviorElement);

	
private:
	//-----Private Data-----
	
	// Entity Base class
	std::string								m_name;
	int										m_defaultHealth = 99999;
	ePhysicsType							m_physicsType = PHYSICS_TYPE_UNASSIGNED;
	bool									m_affectedByGravity = false;
	CollisionDefinition_t					m_collisionDef;

	// AnimatedEntity
	float									m_maxMoveAcceleration = 300.f;
	float									m_maxMoveSpeed = 40.f;
	float									m_maxMoveDeceleration = 100.f;
	float									m_jumpImpulse = 80.f;

	const VoxelAnimationSet*				m_animationSet = nullptr;
	const VoxelSprite*						m_defaultSprite = nullptr;
	bool									m_destructible = false;

	// AIEntity
	std::vector<const BehaviorComponent*>	m_behaviorPrototypes;

	// Item
	ItemSet_t								m_initialItems;

	// Static registry
	static std::map<std::string, const EntityDefinition*> s_definitions;

};
