/************************************************************************/
/* File: Entity.hpp
/* Author: Andrew Chase
/* Date: September 22nd, 2017
/* Description: Abstract class to represent a single voxel object
/************************************************************************/
#pragma once
#include "Engine/Math/Vector3.hpp"
#include "Engine/Rendering/Resources/VoxelTexture.hpp"

class VoxelAnimator;

// Convenience type to know what kind of entity we are
enum eEntityType
{
	ENTITY_TYPE_STATIC,
	ENTITY_TYPE_DYNAMIC,
	ENTITY_TYPE_UNASSIGNED
};

// Team the entity is on
enum eEntityTeam
{
	ENTITY_TEAM_PLAYER,
	ENTITY_TEAM_ENEMY,
	ENTITY_TEAM_UNASSIGNED
};

enum eCollisionShape
{
	COLLISION_SHAPE_NONE,
	COLLISION_SHAPE_DISC,
	COLLISION_SHAPE_BOX,
	NUM_COLLISION_SHAPES
};

enum eCollisionResponse
{
	COLLISION_RESPONSE_NO_CORRECTION,
	COLLISION_RESPONSE_SHARE_CORRECTION,
	COLLISION_RESPONSE_FULL_CORRECTION,
	NUM_COLLISION_RESPONSES
};

struct CollisionDefinition_t
{
	CollisionDefinition_t()
	 : m_shape(COLLISION_SHAPE_DISC), m_response(COLLISION_RESPONSE_FULL_CORRECTION), m_xExtent(4.f), m_zExtent(4.f), m_height(8.f) {}

	CollisionDefinition_t(eCollisionShape shape, eCollisionResponse type, float width, float length, float height)
	: m_shape(shape), m_response(type), m_xExtent(width), m_zExtent(length), m_height(height) {}

	eCollisionShape	m_shape;
	eCollisionResponse	m_response;

	float			m_xExtent;
	float			m_zExtent;
	float			m_height;
};

struct CollisionResult_t
{
	bool collisionOccurred = false;
	Vector3 firstCorrection;
	Vector3 secondCorrection;
};

class Entity
{
public:
	//-----Public Methods-----

	// Initialization
	Entity(eEntityType type);
	virtual ~Entity();

	// Core loop
	virtual void			Update();

	// Mutators
	void					AddPositionOffset(const Vector3& offset);
	void					SetPosition(const Vector3& newPosition);
	void					SetOrientation(float orientation);

	void					TakeDamage(int damageAmount);

	// Accessors
	Vector3					GetEntityPosition() const;
	const VoxelTexture*			GetTextureForOrientation() const;
	CollisionDefinition_t	GetCollisionDefinition() const;

	float					GetMass() const;
	float					GetInverseMass() const;

	// Producers
	bool					IsMarkedForDelete() const;
	Vector3					GetPositionForLocalCoords(const IntVector3& localCoords) const;
	Vector3					GetPositionForLocalIndex(unsigned int index) const;

	IntVector3				GetEntityCoordinatePosition() const;

	// Events
	virtual void			OnCollision(Entity* other);
	virtual void			OnDamageTaken(int damageAmount);
	virtual void			OnDeath();
	virtual void			OnSpawn();

	// Collision	
	void					AddCollisionCorrection(const Vector3& correction);


protected:
	//-----Protected Methods-----


protected:
	//-----Protected Data-----

	Vector3					m_position = Vector3::ZERO;
	float					m_orientation = 0.f;
	IntVector3				m_dimensions;
	bool					m_isMarkedForDelete = false;

	CollisionDefinition_t	m_collisionDef;

	float					m_mass = DEFAULT_MASS;					// Mass of the Entity
	float					m_inverseMass = 1.f / DEFAULT_MASS;		// Cache off inverse for efficiency

	int						m_health = 1;
	eEntityTeam				m_entityTeam = ENTITY_TEAM_UNASSIGNED;
	eEntityType				m_entityType = ENTITY_TYPE_UNASSIGNED;

	VoxelAnimator*			m_animator = nullptr;

	static constexpr float DEFAULT_MASS = 1.0f;

};
