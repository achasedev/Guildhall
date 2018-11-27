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
#include "Engine/Rendering/DebugRendering/DebugRenderSystem.hpp"
#include <stdlib.h>


//-----------------------------------------------------------------------------------------------
// Constructor - takes the definition for its constant data characteristics
//
Entity::Entity(const EntityDefinition* definition)
	: m_definition(definition)
{
	if (m_definition->m_physicsType == PHYSICS_TYPE_DYNAMIC)
	{
		m_physicsComponent = new PhysicsComponent(this);
	}

	// Only create a default texture if the definition has one specified
	if (definition->m_defaultSprite != nullptr && !definition->m_isAnimated)
	{
		m_defaultTexture = definition->m_defaultSprite->GetTextureForOrientation(90.f)->Clone();
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
	
	if (m_defaultTexture != nullptr)
	{
		delete m_defaultTexture;
		m_defaultTexture = nullptr;
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
void Entity::OnVoxelCollision(Entity* other, std::vector<IntVector3> voxelCoords)
{
	if (!m_definition->m_isDestructible || (other != nullptr && !other->GetCollisionDefinition().m_canDestroyVoxels))
	{
		return;
	}

	int numVoxels = (int) voxelCoords.size();

	for (int i = 0; i < numVoxels; ++i)
	{
		IntVector3 currCoords = voxelCoords[i];

		Rgba currColor = m_defaultTexture->GetColorAtCoords(currCoords);

		if (currColor.a > 0)
		{
			Vector3 position = GetPositionForLocalCoords(currCoords);

			Vector3 velocity = Vector3(GetRandomFloatInRange(-1.f, 1.f), 1.0f, GetRandomFloatInRange(-1.f, 1.f));
			velocity = 50.f * velocity.GetNormalized();

			Particle* particle = new Particle(currColor, 10.0f, position, velocity);
			Game::GetWorld()->AddParticle(particle);

			m_defaultTexture->SetColorAtCoords(currCoords, Rgba(0, 0, 0, 0));
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
// Subtracts the amount to the entity's health
//
void Entity::TakeDamage(int damageAmount, const Vector3& knockback /*=Vector3::ZERO*/)
{
	// Apply damage
	m_health -= damageAmount;
	OnDamageTaken(damageAmount);

	// Apply knockback if the entity is dynamic
	if (m_physicsComponent != nullptr)
	{
		m_physicsComponent->AddImpulse(knockback);
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
	Vector3 halfDimensions = Vector3(GetDimensions()) * 0.5f;
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
const VoxelTexture* Entity::GetTextureForRender() const
{
	return m_defaultTexture;
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
// Returns the dimensions of the entity
//
IntVector3 Entity::GetDimensions() const
{
	const VoxelTexture* texture = GetTextureForRender();
	return texture->GetDimensions();
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
// Returns the world bounds for the entity
//
AABB3 Entity::GetWorldBounds() const
{
	Vector3 bottomLeft = m_position;
	Vector3 topRight = m_position + Vector3(GetDimensions());
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
	return m_isPlayer;
}


//-----------------------------------------------------------------------------------------------
// Returns whether the entity is currently standing on something (0 Y velocity with gravity)
//
bool Entity::IsGrounded() const
{
	return m_isGrounded;
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
	IntVector3 dimensions = GetDimensions();

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
