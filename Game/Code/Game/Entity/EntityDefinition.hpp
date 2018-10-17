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

// Shape of the collision boundary
enum eCollisionShape
{
	COLLISION_SHAPE_NONE,
	COLLISION_SHAPE_DISC,
	COLLISION_SHAPE_BOX,
	NUM_COLLISION_SHAPES
};

// How the entity should react to collision corrections
enum eCollisionResponse
{
	COLLISION_RESPONSE_IGNORE_CORRECTION,
	COLLISION_RESPONSE_NO_CORRECTION,
	COLLISION_RESPONSE_SHARE_CORRECTION,
	COLLISION_RESPONSE_FULL_CORRECTION,
	NUM_COLLISION_RESPONSES
};

enum eCollisionTeamException
{
	COLLISION_TEAM_EXCEPTION_NONE,
	COLLISION_TEAM_EXCEPTION_SAME,
	COLLISION_TEAM_EXCEPTION_DIFFERENT,
};

// Collision state for a single entity
struct CollisionDefinition_t
{
	CollisionDefinition_t()
		: m_shape(COLLISION_SHAPE_DISC), m_response(COLLISION_RESPONSE_SHARE_CORRECTION), m_teamException(COLLISION_TEAM_EXCEPTION_NONE)
		, m_xExtent(4.f), m_zExtent(4.f), m_height(8.f), m_collisionLayer(0) {}

	CollisionDefinition_t(eCollisionShape shape, eCollisionResponse response, eCollisionTeamException teamBehavior, float width, float length, float height, unsigned int collisionLayer)
		: m_shape(shape), m_response(response), m_teamException(teamBehavior), m_xExtent(width), m_zExtent(length), m_height(height), m_collisionLayer(collisionLayer) {}

	eCollisionShape				m_shape;
	eCollisionResponse			m_response;
	eCollisionTeamException		m_teamException;
	float						m_xExtent;
	float						m_zExtent;
	float						m_height;
	unsigned int				m_collisionLayer;
};

class VoxelSprite;
class VoxelAnimationSet;
class BehaviorComponent;

class EntityDefinition
{
	friend class Entity;
	friend class MovingEntity;
	friend class Player;

public:
	//-----Public Methods-----
	
	
	std::string	GetName() const;
	BehaviorComponent* CloneBehaviorPrototype(unsigned int index) const;

	bool		HasGravity() const;

	static void						LoadDefinitions(const std::string& filename);
	static const EntityDefinition*	GetDefinition(const std::string& defName);


private:
	//-----Private Methods-----
	
	EntityDefinition(const XMLElement& entityElement);

	BehaviorComponent* ConstructBehaviorPrototype(const XMLElement& behaviorElement);

	
private:
	//-----Private Data-----
	
	std::string					m_name;
	IntVector3					m_dimensions;

	// Movement
	float m_maxMoveAcceleration = 300.f;
	float m_maxMoveSpeed = 40.f;
	float m_maxMoveDeceleration = 100.f;
	float m_jumpImpulse = 80.f;

	// Animation
	const VoxelAnimationSet*	m_animationSet = nullptr;
	const VoxelSprite*			m_defaultSprite = nullptr;

	// Physics
	ePhysicsType				m_physicsType		= PHYSICS_TYPE_UNASSIGNED;
	bool						m_affectedByGravity = false;
	CollisionDefinition_t		m_collisionDef;

	// AI
	std::vector<const BehaviorComponent*> m_behaviorPrototypes;

	// Static registry
	static std::map<std::string, const EntityDefinition*> s_definitions;

};
