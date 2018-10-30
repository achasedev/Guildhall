/************************************************************************/
/* File: Entity.hpp
/* Author: Andrew Chase
/* Date: September 22nd, 2017
/* Description: Abstract class to represent a single voxel object
/************************************************************************/
#pragma once
#include "Game/Entity/EntityDefinition.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Rendering/Resources/VoxelTexture.hpp"

class VoxelAnimator;
class PhysicsComponent;

// Team the entity is on
enum eEntityTeam
{
	ENTITY_TEAM_PLAYER,
	ENTITY_TEAM_ENEMY,
	ENTITY_TEAM_UNASSIGNED
};


class Entity
{
public:
	//-----Public Methods-----

	// Initialization
	Entity(const EntityDefinition* definition);
	virtual ~Entity();

	// Core loop
	virtual void					Update();

	// Mutators
	void							AddPositionOffset(const Vector3& offset);
	void							SetPosition(const Vector3& newPosition);
	void							SetOrientation(float orientation);
	void							SetTeam(eEntityTeam team);

	void							TakeDamage(int damageAmount);

	// Accessors
	Vector3							GetPosition() const;
	Vector3							GetCenterPosition() const;
	Vector3							GetBottomCenterPosition() const;
	float							GetOrientation() const;
	eEntityTeam						GetTeam() const;
	virtual const VoxelTexture*		GetTextureForRender() const;
	CollisionDefinition_t			GetCollisionDefinition() const;
	ePhysicsType					GetPhysicsType() const;
	const EntityDefinition*			GetEntityDefinition() const;
	PhysicsComponent*				GetPhysicsComponent() const;
	virtual IntVector3				GetDimensions() const;
	bool							IsPhysicsEnabled() const;

	float							GetMass() const;
	float							GetInverseMass() const;

	bool							IsPlayer() const;

	// Producers
	bool							IsMarkedForDelete() const;
	Vector3							GetPositionForLocalCoords(const IntVector3& localCoords) const;
	Vector3							GetPositionForLocalIndex(unsigned int index) const;

	IntVector3						GetCoordinatePosition() const;

	// Events
	virtual void					OnCollision(Entity* other);
	virtual void					OnDamageTaken(int damageAmount);
	virtual void					OnDeath();
	virtual void					OnSpawn();

	// Collision	
	void							AddCollisionCorrection(const Vector3& correction);
	void							ApplyCollisionCorrection();

	// Physics 
	void							SetPhysicsEnabled(bool newState);


protected:
	//-----Protected Data-----
	
	// Basic state
	Vector3					m_position = Vector3::ZERO;
	float					m_orientation = 0.f;
	bool					m_isMarkedForDelete = false;
	int						m_health = 9999999;
	eEntityTeam				m_entityTeam = ENTITY_TEAM_UNASSIGNED;

	// Physics
	float					m_mass = DEFAULT_MASS;					// Mass of the Entity
	float					m_inverseMass = 1.f / DEFAULT_MASS;		// Cache off inverse for efficiency
	PhysicsComponent*		m_physicsComponent = nullptr;
	const EntityDefinition* m_definition = nullptr;
	Vector3					m_collisionCorrection = Vector3::ZERO;
	bool					m_physicsEnabled = true;

	bool					m_isPlayer = false;

	VoxelTexture*			m_defaultTexture = nullptr;

	// Statics
	static constexpr float	DEFAULT_MASS = 1.0f;

};
