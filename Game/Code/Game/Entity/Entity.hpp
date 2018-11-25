/************************************************************************/
/* File: Entity.hpp
/* Author: Andrew Chase
/* Date: September 22nd, 2017
/* Description: Abstract class to represent a single voxel object
/************************************************************************/
#pragma once
#include "Game/Entity/EntityDefinition.hpp"
#include "Engine/Math/AABB3.hpp"
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

	void							TakeDamage(int damageAmount, const Vector3& knockback = Vector3::ZERO);

	// Accessors
	int								GetHealth() const;
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
	bool							IsDestructible() const;
	AABB3							GetWorldBounds() const;
	eCollisionLayer					GetCollisionLayer() const;

	float							GetMass() const;
	float							GetInverseMass() const;

	bool							IsPlayer() const;

	// Producers
	bool							IsMarkedForDelete() const;
	bool							IsDynamic() const;
	Vector3							GetPositionForLocalCoords(const IntVector3& localCoords) const;
	Vector3							GetPositionForLocalIndex(unsigned int index) const;
	Vector3							GetForwardVector() const;

	IntVector3						GetCoordinatePosition() const;

	// Events
	virtual void					OnEntityCollision(Entity* other);
	virtual void					OnGroundCollision();
	virtual void					OnVoxelCollision(Entity* other, std::vector<IntVector3> voxelCoords);
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
	int						m_health = 0;
	eEntityTeam				m_entityTeam = ENTITY_TEAM_UNASSIGNED;

	// Physics
	float					m_mass = DEFAULT_MASS;					// Mass of the Entity
	float					m_inverseMass = 1.f / DEFAULT_MASS;		// Cache off inverse for efficiency
	PhysicsComponent*		m_physicsComponent = nullptr;
	const EntityDefinition* m_definition = nullptr;
	Vector3					m_collisionCorrection = Vector3::ZERO;
	bool					m_physicsEnabled = true;

	bool					m_useCollisionLayerOverride = false;
	eCollisionLayer			m_collisionLayerOverride = COLLISION_LAYER_WORLD;

	bool					m_isPlayer = false;

	VoxelTexture*			m_defaultTexture = nullptr;

	// Statics
	static constexpr float	DEFAULT_MASS = 1.0f;

};
