/************************************************************************/
/* File: EntityDefinition.cpp
/* Author: Andrew Chase
/* Date: October 8th, 2018
/* Description: Implementation of the EntityDefinition class
/************************************************************************/
#include "Game/Animation/VoxelSprite.hpp"
#include "Game/Entity/EntityDefinition.hpp"
#include "Game/Animation/VoxelAnimationSet.hpp"
#include "Game/Entity/Components/BehaviorComponent_PursuePath.hpp"
#include "Game/Entity/Components/BehaviorComponent_PursueDirect.hpp"
#include "Game/Entity/Components/BehaviorComponent_PursueJump.hpp"
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


//-----------------------------------------------------------------------------------------------
// Returns the eCollisionShape corresponding to the text
//
eCollisionShape ConvertCollisionShapeFromString(const std::string& shapeName)
{
	if		(shapeName == "disc")	{ return COLLISION_SHAPE_DISC; }
	else if (shapeName == "box")	{ return COLLISION_SHAPE_BOX; }
	else
	{
		return COLLISION_SHAPE_NONE;
	}
}


//-----------------------------------------------------------------------------------------------
// Returns the eCollisionResponse corresponding to the text
//
eCollisionResponse ConvertCollisionResponseFromString(const std::string& responseName)
{
	if		(responseName == "share_correction")	{ return COLLISION_RESPONSE_SHARE_CORRECTION; }
	else if (responseName == "no_correction")		{ return COLLISION_RESPONSE_NO_CORRECTION; }
	else
	{
		return COLLISION_RESPONSE_FULL_CORRECTION;
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

	// Dimensions
	m_dimensions = ParseXmlAttribute(entityElement, "dimensions", IntVector3(8, 8, 8));

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
		m_defaultSprite = VoxelSprite::GetVoxelSprite(defaultSpriteName);
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
			eCollisionShape shape = ConvertCollisionShapeFromString(shapeText);

			std::string responseText = ParseXmlAttribute(*collisionElement, "response", "full_correction");
			eCollisionResponse response = ConvertCollisionResponseFromString(responseText);

			float xExtent	= ParseXmlAttribute(*collisionElement, "xExtent",	4.0f);
			float zExtent	= ParseXmlAttribute(*collisionElement, "zExtent",	4.0f);
			float height	= ParseXmlAttribute(*collisionElement, "height",	8.0f);

			m_collisionDef = CollisionDefinition_t(shape, response, xExtent, zExtent, height);
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
