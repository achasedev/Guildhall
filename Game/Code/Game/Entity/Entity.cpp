/************************************************************************/
/* File: Entity.cpp
/* Author: Andrew Chase
/* Date: September 22nd, 2017
/* Description: Implementation of the Entity class
/************************************************************************/
#include "Game/Entity/Entity.hpp"
#include "Game/Framework/Game.hpp"
#include "Game/Framework/World.hpp"
#include "Game/Entity/Particle.hpp"
#include "Game/Framework/GameCommon.hpp"
#include "Game/Animation/VoxelSprite.hpp"
#include "Game/Entity/Components/PhysicsComponent.hpp"
#include "Engine/Core/Rgba.hpp"
#include "Engine/Assets/AssetDB.hpp"
#include "Engine/Math/MathUtils.hpp"
#include <stdlib.h>


//-----------------------------------------------------------------------------------------------
// Constructor - takes the definition for its constant data characteristics
//
Entity::Entity(const EntityDefinition* definition)
	: m_definition(definition)
{
	ASSERT_OR_DIE(definition != nullptr, "Error: Initializing Entity with null definition");

	if (m_definition->m_physicsType == PHYSICS_TYPE_DYNAMIC)
	{
		m_physicsComponent = new PhysicsComponent(this);
	}

	// Only create a default texture if the definition has one specified
	if (m_definition->m_defaultSprite != nullptr)
	{
		m_defaultSprite = definition->m_defaultSprite->Clone();
	}

	m_health = definition->m_initialHealth;
}


//-----------------------------------------------------------------------------------------------
// Destructor
//
Entity::~Entity()
{
	if (m_physicsComponent != nullptr)
	{
		delete m_physicsComponent;
		m_physicsComponent = nullptr;
	}
	
	if (m_defaultSprite != nullptr)
	{
		delete m_defaultSprite;
		m_defaultSprite = nullptr;
	}
}


//-----------------------------------------------------------------------------------------------
// Update
//
void Entity::Update()
{
}


//-----------------------------------------------------------------------------------------------
// Callback for when collision is detected
//
void Entity::OnEntityCollision(Entity* other)
{
	UNUSED(other);
}


//-----------------------------------------------------------------------------------------------
// Called when the entity makes collision with the ground (falls below it)
//
void Entity::OnGroundCollision()
{
	m_isGrounded = true;
}


//-----------------------------------------------------------------------------------------------
// Callback for individual voxels hit during collision
//
void Entity::OnVoxelCollision(Entity* other, std::vector<IntVector3> relativeVoxelCoords)
{
	if (!m_definition->m_isDestructible || (other != nullptr && !other->GetCollisionDefinition().m_canDestroyVoxels))
	{
		return;
	}

	// Need to remove each voxel relative to our current orientation
	int numVoxels = (int) relativeVoxelCoords.size();
	for (int i = 0; i < numVoxels; ++i)
	{
		IntVector3 currRelativeCoords = relativeVoxelCoords[i];
		
		Rgba currColor = m_defaultSprite->GetColorAtRelativeCoords(currRelativeCoords, m_orientation);

		if (currColor.a > 0)
		{
			Vector3 position = GetPositionForLocalCoords(currRelativeCoords);

			Vector3 velocity = Vector3(GetRandomFloatInRange(-1.f, 1.f), 1.0f, GetRandomFloatInRange(-1.f, 1.f));
			velocity = 50.f * velocity.GetNormalized();

			Particle* particle = new Particle(currColor, 10.0f, position, velocity);
			Game::GetWorld()->AddParticle(particle);

			m_defaultSprite->SetColorAtRelativeCoords(currRelativeCoords, m_orientation, Rgba(0, 0, 0, 0));
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Adds an offset (translates) to the player
//
void Entity::AddPositionOffset(const Vector3& offset)
{
	m_position += offset;
}


//-----------------------------------------------------------------------------------------------
// Hard sets the position of the entity
//
void Entity::SetPosition(const Vector3& newPosition)
{
	m_position = newPosition;
}


//-----------------------------------------------------------------------------------------------
// Sets the orientation of the entity to the provided value
//
void Entity::SetOrientation(float orientation)
{
	m_orientation = orientation;
}


//-----------------------------------------------------------------------------------------------
// Sets the team of the entity to the one specified
//
void Entity::SetTeam(eEntityTeam team)
{
	m_entityTeam = team;
}


//-----------------------------------------------------------------------------------------------
// Sets the flag indicating whether this entity is grounded or not
//
void Entity::SetIsGrounded(bool isGrounded)
{
	m_isGrounded = isGrounded;
}


//-----------------------------------------------------------------------------------------------
// Sets whether this entity should update when the update loop occurs in World
//
void Entity::SetShouldUpdate(bool shouldUpdate)
{
	m_shouldUpdate = shouldUpdate;
}


//-----------------------------------------------------------------------------------------------
// Sets whether this entity should be snapped to the map height when below the terrain
//
void Entity::SetShouldCheckForGroundCollisions(bool shouldCheck)
{
	m_shouldCheckForGroundCollisions = shouldCheck;
}


//-----------------------------------------------------------------------------------------------
// Sets whether this entity is marked for delete or not
//
void Entity::SetMarkedForDelete(bool isMarkedForDelete)
{
	m_isMarkedForDelete = isMarkedForDelete;
}


//-----------------------------------------------------------------------------------------------
// Sets whether this entity should be checked for collisions with the map edges
//
void Entity::SetShouldCheckForEdgeCollisions(bool shouldCheck)
{
	m_shouldCheckForEdgeCollisions = shouldCheck;
}


//-----------------------------------------------------------------------------------------------
// Subtracts the amount to the entity's health
//
void Entity::TakeDamage(int damageAmount, const Vector3& knockback /*=Vector3::ZERO*/)
{
	// Apply damage if not dead
	if (!m_isMarkedForDelete)
	{
		m_health -= damageAmount;
		OnDamageTaken(damageAmount);

		// Apply knockback if the entity is dynamic
		if (m_physicsComponent != nullptr)
		{
			m_physicsComponent->AddImpulse(knockback);
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Returns the current health of the entity
//
int Entity::GetHealth() const
{
	return m_health;
}


//-----------------------------------------------------------------------------------------------
// Called when the entity takes damage, for custom behavior
//
void Entity::OnDamageTaken(int damageAmount)
{
	UNUSED(damageAmount);

	if (m_health <= 0)
	{
		OnDeath();
	}
}


//-----------------------------------------------------------------------------------------------
// Called when the entity's health is <= 0
//
void Entity::OnDeath()
{
	m_isMarkedForDelete = true;
}


//-----------------------------------------------------------------------------------------------
// Called when the entity is spawned
//
void Entity::OnSpawn()
{
}


//-----------------------------------------------------------------------------------------------
// Adds the correction to the entity's position to fix a collision
//
void Entity::AddCollisionCorrection(const Vector3& correction)
{
	m_collisionCorrection += correction;
}


//-----------------------------------------------------------------------------------------------
// Applies the net collision correction for the current frame
//
void Entity::ApplyCollisionCorrection()
{
	m_position += m_collisionCorrection;
	m_collisionCorrection = Vector3::ZERO;
}


//-----------------------------------------------------------------------------------------------
// Sets whether the physics step should be applied to this entity
//
void Entity::SetPhysicsEnabled(bool newState)
{
	m_physicsEnabled = newState;
}


//-----------------------------------------------------------------------------------------------
// Returns the world position of the entity
//
Vector3 Entity::GetPosition() const
{
	return m_position;
}


//-----------------------------------------------------------------------------------------------
// Returns the floating point position of the entity's absolute center
//
Vector3 Entity::GetCenterPosition() const
{
	Vector3 halfDimensions = Vector3(GetOrientedDimensions()) * 0.5f;
	return m_position + halfDimensions;
}


//-----------------------------------------------------------------------------------------------
// Returns the center position of the entity at its bottom
//
Vector3 Entity::GetBottomCenterPosition() const
{
	Vector3 center = GetCenterPosition();
	center.y = m_position.y;
	return center;
}


//-----------------------------------------------------------------------------------------------
// Returns the orientation of the entity
//
float Entity::GetOrientation() const
{
	return m_orientation;
}


//-----------------------------------------------------------------------------------------------
// Returns the team index of the entity
//
eEntityTeam Entity::GetTeam() const
{
	return m_entityTeam;
}


//-----------------------------------------------------------------------------------------------
// Returns the 3D texture to used for rendering, based on the current 2D orientation of the entity
//
const VoxelSprite* Entity::GetVoxelSprite() const
{
	return m_defaultSprite;
}


//-----------------------------------------------------------------------------------------------
// Returns the collision definition of this entity
//
CollisionDefinition_t Entity::GetCollisionDefinition() const
{
	return m_definition->m_collisionDef;
}


//-----------------------------------------------------------------------------------------------
// Returns the physics type of the entity
//
ePhysicsType Entity::GetPhysicsType() const
{
	return m_definition->m_physicsType;
}


//-----------------------------------------------------------------------------------------------
// Returns the definition used to describe this entity's data characteristics
//
const EntityDefinition* Entity::GetEntityDefinition() const
{
	return m_definition;
}


//-----------------------------------------------------------------------------------------------
// Returns the physics component of this entity
//
PhysicsComponent* Entity::GetPhysicsComponent() const
{
	return m_physicsComponent;
}


//-----------------------------------------------------------------------------------------------
// Returns whether this entity should have the physics step applied to it
//
bool Entity::IsPhysicsEnabled() const
{
	return m_physicsEnabled;
}


//-----------------------------------------------------------------------------------------------
// Returns whether this entity can have voxels broken off of it
//
bool Entity::IsDestructible() const
{
	return m_definition->m_isDestructible;
}


//-----------------------------------------------------------------------------------------------
// Returns the dimensions of the entity, taking into consideration its current orientation 
//
IntVector3 Entity::GetOrientedDimensions() const
{
	return GetVoxelSprite()->GetOrientedDimensions(m_orientation);
}


//-----------------------------------------------------------------------------------------------
// Returns the world bounds for the entity
//
AABB3 Entity::GetWorldBounds() const
{
	Vector3 bottomLeft = m_position;
	Vector3 topRight = m_position + Vector3(GetVoxelSprite()->GetOrientedDimensions(m_orientation));
	return AABB3(bottomLeft, topRight);
}


//-----------------------------------------------------------------------------------------------
// Returns the layer that this entity currently is on for collisions
//
eCollisionLayer Entity::GetCollisionLayer() const
{
	if (m_useCollisionLayerOverride)
	{
		return m_collisionLayerOverride;
	}

	return m_definition->m_collisionDef.layer;
}


//-----------------------------------------------------------------------------------------------
// Returns the mass of the entity
//
float Entity::GetMass() const
{
	return m_mass;
}


//-----------------------------------------------------------------------------------------------
// Returns one over the mass of the entity
//
float Entity::GetInverseMass() const
{
	return m_inverseMass;
}


//-----------------------------------------------------------------------------------------------
// Returns whether this entity is a player entity
//
bool Entity::IsPlayer() const
{
	return m_definition->m_entityClass == ENTITY_CLASS_PLAYER;
}


//-----------------------------------------------------------------------------------------------
// Returns whether the entity is currently standing on something (0 Y velocity with gravity)
//
bool Entity::IsGrounded() const
{
	return m_isGrounded;
}


//-----------------------------------------------------------------------------------------------
// Returns true if this entity should update when the world updates
//
bool Entity::IsSetToUpdate() const
{
	return m_shouldUpdate;
}


//-----------------------------------------------------------------------------------------------
// Returns true if the entity should be checked for ground collisions
//
bool Entity::ShouldCheckForGroundCollisions() const
{
	return m_shouldCheckForGroundCollisions;
}


//-----------------------------------------------------------------------------------------------
// Returns true if this entity should be checked for edge collisions
//
bool Entity::ShouldCheckForEdgeCollisions() const
{
	return m_shouldCheckForEdgeCollisions;
}


//-----------------------------------------------------------------------------------------------
// Returns whether this entity has been marked to be deleted at the end of the frame
//
bool Entity::IsMarkedForDelete() const
{
	return m_isMarkedForDelete;
}


//-----------------------------------------------------------------------------------------------
// Returns whether this entity has dynamic physics or not (has the physics step applied)
//
bool Entity::IsDynamic() const
{
	return (m_definition->m_physicsType == PHYSICS_TYPE_DYNAMIC);
}


//-----------------------------------------------------------------------------------------------
// Returns the position of the entity's voxel at the given local coords
//
Vector3 Entity::GetPositionForLocalCoords(const IntVector3& localCoords) const
{
	Vector3 bottomLeft = m_position;
	Vector3 voxelPosition = Vector3(bottomLeft + Vector3(localCoords));

	return voxelPosition;
}


//-----------------------------------------------------------------------------------------------
// Returns the position of the entity's voxel at the given index
//
Vector3 Entity::GetPositionForLocalIndex(unsigned int index) const
{
	IntVector3 dimensions = GetOrientedDimensions();

	int y = index / (dimensions.x * dimensions.z);
	int leftOver = index % (dimensions.x * dimensions.z);

	int z = leftOver / (dimensions.x);
	int x = leftOver % (dimensions.x);

	return GetPositionForLocalCoords(IntVector3(x, y, z));
}


//-----------------------------------------------------------------------------------------------
// Returns the vector representing the direction this entity is currently facing
//
Vector3 Entity::GetForwardVector() const
{
	Vector2 direction = Vector2::MakeDirectionAtDegrees(m_orientation);
	return Vector3(direction.x, 0.f, direction.y);
}


//-----------------------------------------------------------------------------------------------
// Returns the coordinate position that this entity occupies
//
IntVector3 Entity::GetCoordinatePosition() const
{
	return IntVector3(RoundToNearestInt(m_position.x), RoundToNearestInt(m_position.y), RoundToNearestInt(m_position.z));
}
