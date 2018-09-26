/************************************************************************/
/* File: Entity.hpp
/* Author: Andrew Chase
/* Date: September 22nd, 2017
/* Description: Abstract class to represent a single voxel object
/************************************************************************/
#pragma once
#include "Engine/Math/Vector3.hpp"
#include "Engine/Rendering/Resources/Texture3D.hpp"

// Direction the entity is facing, used for rendering
enum eFacingDirection
{
	DIRECTION_NORTH,
	DIRECTION_EAST,
	DIRECTION_SOUTH,
	DIRECTION_WEST, 
	NUM_DIRECTIONS
};

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

enum eCollisionType
{
	COLLISION_TYPE_DISC,
	COLLISION_TYPE_BOX,
	NUM_COLLISION_TYPES
};

struct CollisionDefinition_t
{
	CollisionDefinition_t()
	 : m_type(COLLISION_TYPE_DISC), m_width(4.f), m_length(4.f), m_height(4.f) {}

	CollisionDefinition_t(eCollisionType type, float width, float length, float height)
	: m_type(type), m_width(width), m_length(length), m_height(height) {}

	eCollisionType	m_type;
	float			m_width;
	float			m_length;
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
	Vector3					GetPosition() const;
	Texture3D*				GetTextureForOrientation() const;
	float					GetCollisionRadius() const;
	CollisionDefinition_t	GetCollisionDefinition() const;

	float					GetMass() const;
	float					GetInverseMass() const;

	// Producers
	bool					IsMarkedForDelete() const;


	// Events
	virtual void			OnCollision(Entity* other);
	virtual void			OnDamageTaken(int damageAmount);
	virtual void			OnDeath();
	virtual void			OnSpawn();

	// Collision	
	void					AddCollisionCorrection(const Vector3& correction);


protected:
	//-----Protected Methods-----

	virtual void			SetupVoxelTextures(const char* filename);


protected:
	//-----Protected Data-----

	Vector3					m_position = Vector3::ZERO;
	float					m_orientation = 0.f;
	bool					m_isMarkedForDelete = false;

	CollisionDefinition_t	m_collisionDef;

	//float					m_collisionRadius = 4.f;

	float					m_mass = DEFAULT_MASS;					// Mass of the Entity
	float					m_inverseMass = 1.f / DEFAULT_MASS;		// Cache off inverse for efficiency

	int						m_health = 1;
	eEntityTeam				m_entityTeam = ENTITY_TEAM_UNASSIGNED;
	eEntityType				m_entityType = ENTITY_TYPE_UNASSIGNED;
	Texture3D*				m_textures[NUM_DIRECTIONS];

	static constexpr float DEFAULT_MASS = 1.0f;

};
