/************************************************************************/
/* File: EntityDefinition.cpp
/* Author: Andrew Chase
/* Date: October 8th, 2018
/* Description: Implementation of the EntityDefinition class
/************************************************************************/
#include "Game/Animation/VoxelSprite.hpp"
#include "Game/Entity/EntityDefinition.hpp"
#include "Game/Animation/VoxelAnimationSet.hpp"
#include "Game/Entity/Components/BehaviorComponent_Charge.hpp"
#include "Game/Entity/Components/BehaviorComponent_PursueJump.hpp"
#include "Game/Entity/Components/BehaviorComponent_ShootDirect.hpp"
#include "Game/Entity/Components/BehaviorComponent_ShootCircle.hpp"
#include "Game/Entity/Components/BehaviorComponent_PursueDirect.hpp"
#include "Engine/Core/Utility/StringUtils.hpp"
#include "Engine/Core/Utility/ErrorWarningAssert.hpp"

// Global map for all entity definitions
std::map<std::string, const EntityDefinition*> EntityDefinition::s_definitions;


//-----------------------------------------------------------------------------------------------
// Returns the ePhysicsType corresponding to the text
//
ePhysicsType ConvertPhysicsTypeFromString(const std::string& physicsTypeName)
{
	if		(physicsTypeName == "static")	{ return PHYSICS_TYPE_STATIC; }
	else if (physicsTypeName == "dynamic")	{ return PHYSICS_TYPE_DYNAMIC; }
	else
	{
		return PHYSICS_TYPE_UNASSIGNED;
	}
}


//- C FUNCTION ----------------------------------------------------------------------------------
// Converts the string name of a collision layer into the corresponding enum
//
eCollisionLayer ConvertCollisionLayerFromString(const std::string& layerName)
{
	if		(layerName == "world")			{ return COLLISION_LAYER_WORLD; }
	else if (layerName == "player")			{ return COLLISION_LAYER_PLAYER; }
	else if (layerName == "enemy")			{ return COLLISION_LAYER_ENEMY; }
	else if (layerName == "player_bullet")	{ return COLLISION_LAYER_PLAYER_BULLET; }
	else if (layerName == "enemy_bullet")	{ return COLLISION_LAYER_ENEMY_BULLET; }
	else if (layerName == "item")			{ return COLLISION_LAYER_ITEM; }
	else
	{
		return COLLISION_LAYER_WORLD; // Default to layer that collides with everything
	}
}


//-----------------------------------------------------------------------------------------------
// Returns the eCollisionResponse corresponding to the text
//
eCorrectionResponse ConvertCollisionResponseFromString(const std::string& responseName)
{
	if		(responseName == "share_correction")	{ return COLLISION_RESPONSE_SHARE_CORRECTION; }
	else if (responseName == "no_correction")		{ return COLLISION_RESPONSE_NO_CORRECTION; }
	else if (responseName == "ignore_correction")	{ return COLLISION_RESPONSE_IGNORE_CORRECTION; }
	else if (responseName == "full_correction")		{ return COLLISION_RESPONSE_FULL_CORRECTION; }
	else
	{
		ERROR_AND_DIE(Stringf("Invalid collision response: %s", responseName.c_str()));
	}
}


//-----------------------------------------------------------------------------------------------
// Constructor
//
EntityDefinition::EntityDefinition(const XMLElement& entityElement)
{
	// Name
	m_name = ParseXmlAttribute(entityElement, "name");
	ASSERT_OR_DIE(m_name.size() > 0, "Error: EntityDefinition lacks a name");

	m_id = ParseXmlAttribute(entityElement, "id", m_id);
	ASSERT_OR_DIE(m_id >= 0, "Error: EntityDefinition lacks an ID");

	m_initialHealth = ParseXmlAttribute(entityElement, "initial_health", m_initialHealth);

	// Movement
	const XMLElement* moveElement = entityElement.FirstChildElement("Movement");
	if (moveElement != nullptr)
	{
		m_maxMoveAcceleration	= ParseXmlAttribute(*moveElement, "max_acceleration",	m_maxMoveAcceleration);
		m_maxMoveSpeed			= ParseXmlAttribute(*moveElement, "max_speed",			m_maxMoveSpeed);
		m_maxMoveDeceleration	= ParseXmlAttribute(*moveElement, "max_deceleration",	m_maxMoveDeceleration);
		m_jumpImpulse			= ParseXmlAttribute(*moveElement, "jump_impulse",		m_jumpImpulse);
	}

	// Visuals
	const XMLElement* visualElement = entityElement.FirstChildElement("Visuals");
	if (visualElement != nullptr)
	{
		std::string animSetName = ParseXmlAttribute(*visualElement, "animation_set", "");
		m_animationSet = VoxelAnimationSet::GetAnimationSet(animSetName);

		if (m_animationSet != nullptr)
		{
			m_isAnimated = true;
		}

		std::string defaultSpriteName = ParseXmlAttribute(*visualElement, "default_sprite", "default");
		m_defaultSprite = VoxelSprite::CreateVoxelSpriteClone(defaultSpriteName);

		m_isDestructible = ParseXmlAttribute(*visualElement, "destructible", false);

		GUARANTEE_OR_DIE(!(m_isAnimated == true && m_isDestructible == true), Stringf("Error: Entity \"%s\" is destructible and animated, not allowed!", m_name.c_str()));
		GUARANTEE_OR_DIE(m_defaultSprite != nullptr, Stringf("Error: Default sprite not found for entity \"%s\"", m_name.c_str()));
	}

	// Physics
	const XMLElement* physicsElement = entityElement.FirstChildElement("Physics");
	if (physicsElement != nullptr)
	{
		std::string physicsTypeName = ParseXmlAttribute(*physicsElement, "physics_type", "dynamic");
		m_physicsType = ConvertPhysicsTypeFromString(physicsTypeName);

		m_affectedByGravity = ParseXmlAttribute(*physicsElement, "has_gravity", m_affectedByGravity);

		// Collision Definition
		const XMLElement* collisionElement = physicsElement->FirstChildElement("CollisionDefinition");

		if (collisionElement != nullptr)
		{
			std::string responseText = ParseXmlAttribute(*collisionElement, "response", "full_correction");
			eCorrectionResponse response = ConvertCollisionResponseFromString(responseText);

			std::string layerName = ParseXmlAttribute(*collisionElement, "layer", "");
			eCollisionLayer layer = ConvertCollisionLayerFromString(layerName);
			bool canDestroyVoxels = ParseXmlAttribute(*collisionElement, "can_destroy_voxels", m_collisionDef.m_canDestroyVoxels);
			float collisionKnockback = ParseXmlAttribute(*collisionElement, "knockback", m_collisionDef.m_collisionKnockback);

			m_collisionDef = CollisionDefinition_t(response, layer, canDestroyVoxels, collisionKnockback);
		}
	}

	// AI
	const XMLElement* aiElement = entityElement.FirstChildElement("AI");
	if (aiElement != nullptr)
	{
		m_behaviorPrototype = ConstructBehaviorPrototype(*aiElement);
	}

	// Projectiles
	const XMLElement* projElement = entityElement.FirstChildElement("Projectile");
	if (projElement != nullptr)
	{
		m_projectileSpeed		= ParseXmlAttribute(*projElement, "speed",		m_projectileSpeed);
		m_projectileLifetime	= ParseXmlAttribute(*projElement, "lifetime",	m_projectileLifetime);
		m_projectileDamage		= ParseXmlAttribute(*projElement, "damage",		m_projectileDamage);
		m_projectileHitRadius	= ParseXmlAttribute(*projElement, "hit_radius", m_projectileHitRadius);
	}

	// Weapons
	const XMLElement* weaponElement = entityElement.FirstChildElement("Weapon");
	if (weaponElement != nullptr)
	{
		std::string projectileName = ParseXmlAttribute(*weaponElement, "projectile", "bullet");
		m_projectileDefinition = EntityDefinition::GetDefinition(projectileName);

		m_fireRate = ParseXmlAttribute(*weaponElement, "fire_rate", m_fireRate);
		m_fireSpread = ParseXmlAttribute(*weaponElement, "fire_spread", m_fireSpread);
		m_projectilesFiredPerShot = ParseXmlAttribute(*weaponElement, "count_per_shot", m_projectilesFiredPerShot);
		m_initialAmmoCount = ParseXmlAttribute(*weaponElement, "initial_ammo", m_initialAmmoCount);
	}

	// Character Select Hack
	const XMLElement* characterElement = entityElement.FirstChildElement("CharacterSelect");
	if (characterElement != nullptr)
	{
		std::string playerDefName = ParseXmlAttribute(*characterElement, "player_definition", "");
		m_playerCharacterDefinition = EntityDefinition::GetDefinition(playerDefName);
	}
}


//-----------------------------------------------------------------------------------------------
// Constructs and returns a behavior component subclass given the element
// Used for prototype construction on entity construction
//
BehaviorComponent* EntityDefinition::ConstructBehaviorPrototype(const XMLElement& behaviorElement)
{
	BehaviorComponent* toReturn;

	std::string behaviorName = ParseXmlAttribute(behaviorElement, "behavior", "");

	if (behaviorName == "pursue_direct")
	{
		toReturn = new BehaviorComponent_PursueDirect();
	}
	else if (behaviorName == "pursue_jump")
	{
		toReturn = new BehaviorComponent_PursueJump();
	}
	else if (behaviorName == "charge")
	{
		BehaviorComponent_Charge* chargeBehavior = new BehaviorComponent_Charge();
		chargeBehavior->m_chargeDuration = ParseXmlAttribute(behaviorElement, "charge_duration", chargeBehavior->m_chargeDuration);
		chargeBehavior->m_restDuration = ParseXmlAttribute(behaviorElement, "rest_duration", chargeBehavior->m_restDuration);
		chargeBehavior->m_chargeSpeed = ParseXmlAttribute(behaviorElement, "charge_speed", chargeBehavior->m_chargeSpeed);
		chargeBehavior->m_damageOnTouch = ParseXmlAttribute(behaviorElement, "damage_on_touch", chargeBehavior->m_damageOnTouch);
		chargeBehavior->m_damageOnCharge = ParseXmlAttribute(behaviorElement, "damage_on_charge", chargeBehavior->m_damageOnCharge);
		chargeBehavior->m_knockbackMagnitude = ParseXmlAttribute(behaviorElement, "knockback", chargeBehavior->m_knockbackMagnitude);

		toReturn = chargeBehavior;
	}
	else if (behaviorName == "shoot_direct")
	{
		std::string weaponDefName = ParseXmlAttribute(behaviorElement, "weapon", "Pistol");
		const EntityDefinition* weaponDefinition = EntityDefinition::GetDefinition(weaponDefName);
		ASSERT_OR_DIE(weaponDefinition != nullptr, Stringf("Error: Bad weapon name in behavior element, \"%s\"", weaponDefName.c_str()));
		
		BehaviorComponent_ShootDirect* shootBehavior = new BehaviorComponent_ShootDirect(weaponDefinition);
		shootBehavior->m_shootRange = ParseXmlAttribute(behaviorElement, "shoot_range", shootBehavior->m_shootRange);

		toReturn = shootBehavior;
	}
	else if (behaviorName == "shoot_circle")
	{
		std::string weaponDefName = ParseXmlAttribute(behaviorElement, "weapon", "Pistol");
		const EntityDefinition* weaponDefinition = EntityDefinition::GetDefinition(weaponDefName);
		ASSERT_OR_DIE(weaponDefinition != nullptr, Stringf("Error: Bad weapon name in behavior element, \"%s\"", weaponDefName.c_str()));

		BehaviorComponent_ShootCircle* shootBehavior = new BehaviorComponent_ShootCircle(weaponDefinition);
		shootBehavior->m_shootRange = ParseXmlAttribute(behaviorElement, "shoot_range", shootBehavior->m_shootRange);

		toReturn = shootBehavior;
	}
	else
	{
		ERROR_AND_DIE("Unknown Behavior");
	}

	return toReturn;
}


//-----------------------------------------------------------------------------------------------
// Returns the name of the definition
//
std::string EntityDefinition::GetName() const
{
	return m_name;
}


//-----------------------------------------------------------------------------------------------
// Returns the (x,y,z) dimensions of the entity
//
IntVector3 EntityDefinition::GetDimensions() const
{
	return m_defaultSprite->GetBaseDimensions();
}


//-----------------------------------------------------------------------------------------------
// Returns a behavior component clone of the prototype at the given index
//
BehaviorComponent* EntityDefinition::CloneBehaviorPrototype() const
{
	return m_behaviorPrototype->Clone();
}


//-----------------------------------------------------------------------------------------------
// Returns whether this entity is affected by gravity (only applies for dynamic entities)
//
bool EntityDefinition::HasGravity() const
{
	return m_affectedByGravity;
}


//-----------------------------------------------------------------------------------------------
// Returns the default (max) health of this entity type
//
int EntityDefinition::GetInitialHealth() const
{
	return m_initialHealth;
}


//-----------------------------------------------------------------------------------------------
// Returns the default sprite for this definition
//
const VoxelSprite* EntityDefinition::GetDefaultSprite() const
{
	return m_defaultSprite;
}


//-----------------------------------------------------------------------------------------------
// Loads the entity definitions specified in the file filename
//
void EntityDefinition::LoadDefinitions(const std::string& filename)
{
	XMLDocument document;
	XMLError error = document.LoadFile(filename.c_str());
	ASSERT_OR_DIE(error == tinyxml2::XML_SUCCESS, Stringf("Error: EntityDefinition::LoadDefinitions() couldn't open file %s", filename.c_str()).c_str());

	const XMLElement* rootElement = document.RootElement();
	ASSERT_OR_DIE(rootElement != nullptr, Stringf("Error: EntityDefinition::LoadDefinitions() loaded file with no root element, file: %s", filename.c_str()).c_str());

	const XMLElement* defElement = rootElement->FirstChildElement();

	while (defElement != nullptr)
	{
		const EntityDefinition* newDefinition = new EntityDefinition(*defElement);
		s_definitions[newDefinition->m_name] = newDefinition;

		defElement = defElement->NextSiblingElement();
	}
}


//-----------------------------------------------------------------------------------------------
// Returns the definition given by defName if it exists, or nullptr otherwise
//
const EntityDefinition* EntityDefinition::GetDefinition(const std::string& defName)
{
	bool exists = s_definitions.find(defName) != s_definitions.end();

	if (exists)
	{
		return s_definitions.at(defName);
	}

	return nullptr;
}


//-----------------------------------------------------------------------------------------------
// Returns the entity definition with the given ID, nullptr if it doesn't exist
//
const EntityDefinition* EntityDefinition::GetDefinition(int id)
{
	std::map<std::string, const EntityDefinition*>::const_iterator itr = s_definitions.begin();

	for (itr; itr != s_definitions.end(); ++itr)
	{
		if (itr->second->m_id == id)
		{
			return itr->second;
		}
	}

	return nullptr;
}
