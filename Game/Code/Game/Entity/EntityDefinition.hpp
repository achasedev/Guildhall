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
	CollisionDefinition_t() {}
	CollisionDefinition_t(eCollisionResponse response, eCollisionTeamException teamBehavior, unsigned int collisionLayer)
		: m_response(response), m_teamException(teamBehavior), m_collisionLayer(collisionLayer) {}

	eCollisionResponse			m_response = COLLISION_RESPONSE_SHARE_CORRECTION;
	eCollisionTeamException		m_teamException = COLLISION_TEAM_EXCEPTION_NONE;
	unsigned int				m_collisionLayer = 0;
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
	IntVector3 GetDimensions() const;
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
