/************************************************************************/
/* File: EntityDefinition.cpp
/* Author: Andrew Chase
/* Date: October 8th, 2018
/* Description: Implementation of the EntityDefinition class
/************************************************************************/
#include "Game/Animation/VoxelSprite.hpp"
#include "Game/Entity/EntityDefinition.hpp"
#include "Game/Animation/VoxelAnimationSet.hpp"
#include "Game/Entity/Components/BehaviorComponent_Shoot.hpp"
#include "Game/Entity/Components/BehaviorComponent_PursuePath.hpp"
#include "Game/Entity/Components/BehaviorComponent_PursueJump.hpp"
#include "Game/Entity/Components/BehaviorComponent_PursueDirect.hpp"
#include "Engine/Core/Utility/StringUtils.hpp"
#include "Engine/Core/Utility/ErrorWarningAssert.hpp"
#include "Engine/Rendering/Resources/VoxelTexture.hpp"

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


//-----------------------------------------------------------------------------------------------
// Returns the eCollisionResponse corresponding to the text
//
eCollisionResponse ConvertCollisionResponseFromString(const std::string& responseName)
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
// Returns the eCollisionTeamException corresponding to the text
//
eCollisionTeamException ConvertCollisionTeamExceptionFromString(const std::string& exceptionName)
{
	if (exceptionName == "none") { return COLLISION_TEAM_EXCEPTION_NONE; }
	else if (exceptionName == "same") { return COLLISION_TEAM_EXCEPTION_SAME; }
	else if (exceptionName == "different") { return COLLISION_TEAM_EXCEPTION_DIFFERENT; }
	else
	{
		ERROR_AND_DIE(Stringf("Invalid collision team exception: %s", exceptionName.c_str()));
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

	// Movement
	const XMLElement* moveElement = entityElement.FirstChildElement("Movement");
	if (moveElement != nullptr)
	{
		m_maxMoveAcceleration	= ParseXmlAttribute(*moveElement, "maxAcceleration",	m_maxMoveAcceleration);
		m_maxMoveSpeed			= ParseXmlAttribute(*moveElement, "maxSpeed",			m_maxMoveSpeed);
		m_maxMoveDeceleration	= ParseXmlAttribute(*moveElement, "maxDeceleration",	m_maxMoveDeceleration);
		m_jumpImpulse			= ParseXmlAttribute(*moveElement, "jumpImpulse",		m_jumpImpulse);
	}

	// Animation
	const XMLElement* animElement = entityElement.FirstChildElement("Animation");
	if (animElement != nullptr)
	{
		std::string animSetName = ParseXmlAttribute(*animElement, "set", "");
		m_animationSet = VoxelAnimationSet::GetAnimationSet(animSetName);

		std::string defaultSpriteName = ParseXmlAttribute(*animElement, "defaultSprite", "default");
		m_defaultSprite = VoxelSprite::CreateVoxelSpriteClone(defaultSpriteName);

		ASSERT_OR_DIE(m_defaultSprite != nullptr, "Error: Default sprite not found");
	}

	// Physics
	const XMLElement* physicsElement = entityElement.FirstChildElement("Physics");
	if (physicsElement != nullptr)
	{
		std::string physicsTypeName = ParseXmlAttribute(*physicsElement, "physicsType", "dynamic");
		m_physicsType = ConvertPhysicsTypeFromString(physicsTypeName);

		m_affectedByGravity = ParseXmlAttribute(*physicsElement, "hasGravity", false);

		// Collision Definition
		const XMLElement* collisionElement = physicsElement->FirstChildElement("CollisionDefinition");

		if (collisionElement != nullptr)
		{
			std::string shapeText = ParseXmlAttribute(*collisionElement, "shape", "disc");

			std::string responseText = ParseXmlAttribute(*collisionElement, "response", "full_correction");
			eCollisionResponse response = ConvertCollisionResponseFromString(responseText);

			std::string teamExceptionText = ParseXmlAttribute(*collisionElement, "teamException", "none");
			eCollisionTeamException teamException = ConvertCollisionTeamExceptionFromString(teamExceptionText);

			int layer = ParseXmlAttribute(*collisionElement, "layer", 0);

			m_collisionDef = CollisionDefinition_t(response, teamException, (unsigned int) layer);
		}
	}

	// AI
	const XMLElement* aiElement = entityElement.FirstChildElement("AI");
	if (aiElement != nullptr)
	{
		const XMLElement* behaviorElement = aiElement->FirstChildElement();

		while (behaviorElement != nullptr)
		{
			BehaviorComponent* prototype = ConstructBehaviorPrototype(*behaviorElement);
			m_behaviorPrototypes.push_back(prototype);

			behaviorElement = behaviorElement->NextSiblingElement();
		}	
	}
}


//-----------------------------------------------------------------------------------------------
// Constructs and returns a behavior component subclass given the element
// Used for prototype construction on entity construction
//
BehaviorComponent* EntityDefinition::ConstructBehaviorPrototype(const XMLElement& behaviorElement)
{
	BehaviorComponent* toReturn;

	std::string behaviorName = behaviorElement.Name();

	if (behaviorName == "PursuePath")
	{
		toReturn = new BehaviorComponent_PursuePath();
	}
	else if (behaviorName == "PursueDirect")
	{
		toReturn = new BehaviorComponent_PursueDirect();
	}
	else if (behaviorName == "PursueJump")
	{
		toReturn = new BehaviorComponent_PursueJump();
	}
	else if (behaviorName == "Shoot")
	{
		std::string projectileDefName = ParseXmlAttribute(behaviorElement, "projectile", "Bullet");
		const EntityDefinition* projDef = EntityDefinition::GetDefinition(projectileDefName);
		ASSERT_OR_DIE(projDef != nullptr, Stringf("Error: Bad projectile name in behavior element, \"%s\"", projectileDefName.c_str()));
		
		float fireRate = ParseXmlAttribute(behaviorElement, "fireRate", 1.0f);

		toReturn = new BehaviorComponent_Shoot(projDef, fireRate);
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
	return m_defaultSprite->GetDimensions();
}


//-----------------------------------------------------------------------------------------------
// Returns a behavior component clone of the prototype at the given index
//
BehaviorComponent* EntityDefinition::CloneBehaviorPrototype(unsigned int index) const
{
	return m_behaviorPrototypes[index]->Clone();
}


//-----------------------------------------------------------------------------------------------
// Returns whether this entity is affected by gravity (only applies for dynamic entities)
//
bool EntityDefinition::HasGravity() const
{
	return m_affectedByGravity;
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
