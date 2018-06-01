/************************************************************************/
/* File: Entity.cpp
/* Author: Andrew Chase
/* Date: November 14th, 2017
/* Bugs: None
/* Description: Implementation of the Entity class
/************************************************************************/
#include "Game/Entity.hpp"
#include "Game/EntityDefinition.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Map.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/SpriteAnimSet.hpp"
#include "Engine/Renderer/SpriteAnim.hpp"
#include "Engine/Math/MathUtils.hpp"


//-----------------------------------------------------------------------------------------------
// Base constructor for the entity, for now just sets to default values
//
Entity::Entity(const Vector2& position, float orientation, const EntityDefinition* definition, Map* entityMap, const std::string& name)
	: m_name(name)
	, m_position(position)
	, m_map(entityMap)
	, m_velocity(Vector2::ZERO)
	, m_force(Vector2::ZERO)
	, m_orientationDegrees(orientation)
	, m_angularVelocity(0.f)
	, m_health(definition->GetStartHealth())
	, m_age(0.f)
	, m_ageAtDeath(-1.f)
	, m_isMarkedForDeletion(false)
	, m_isUsingPhysics(true)
	, m_spriteAnimSet(new SpriteAnimSet(definition->GetAnimSetDefinition()))
	, m_entityDefinition(definition)
{
	InitializeBaseStats(definition);
}


//-----------------------------------------------------------------------------------------------
// Destructor - deletes the SpriteAnimSet
//
Entity::~Entity()
{
	delete m_spriteAnimSet;
}


//-----------------------------------------------------------------------------------------------
// Updated the entity's data, called each frame
//
void Entity::Update(float deltaTime)
{
	// Translate the Entity based on its velocity and time from the last frame, also applying force
	m_position += ((m_velocity + m_force) * deltaTime);

	// Decrement the force over time
	float forceMagnitude = (m_force == Vector2::ZERO ? 0.f : m_force.NormalizeAndGetLength());
	forceMagnitude -= (GLOBAL_FRICTION * deltaTime);

	if (forceMagnitude < 0.2f)
	{
		forceMagnitude = 0.f;
	}

	m_force *= forceMagnitude;
	
	// Rotate the Entity based on its angular velocity
	m_orientationDegrees += (m_angularVelocity * deltaTime);

	// Ensure wrap around if the degree measure goes over 360
	while (m_orientationDegrees >= 360.f)
	{
		m_orientationDegrees -= 360.f;
	}

	// Update the Entity's age
	m_age += deltaTime;

	// Update the sprite animation
	m_spriteAnimSet->Update(deltaTime);
}


//-----------------------------------------------------------------------------------------------
// Draws entity-related graphics
//
void Entity::Render() const
{
	// Draw the animation set's current animation
	const Texture& texture = m_spriteAnimSet->GetTexture();
	AABB2 currentUVs = m_spriteAnimSet->GetCurrentUVs();
	bool shouldOrient = m_spriteAnimSet->GetCurrentAnimation()->ShouldAutoOrient();

	// Set up the coordinate system
	g_theRenderer->PushMatrix();
	g_theRenderer->TranslateCoordinateSystem2D(m_position);
	if (shouldOrient)
	{
		g_theRenderer->RotateCoordinateSystem2D(m_orientationDegrees);
	}

	g_theRenderer->DrawTexturedAABB2(m_entityDefinition->GetLocalDrawBounds(), texture, currentUVs, Rgba::WHITE);

	g_theRenderer->PopMatrix();


	// Draw the physics radius if in Debug Mode
	if (g_inDebugMode)
	{
		g_theRenderer->DrawDottedCircle(m_position, m_entityDefinition->GetInnerRadius(), Rgba::CYAN);
	}
}


//-----------------------------------------------------------------------------------------------
// Sets the 2D world position of the object
//
void Entity::SetPosition(const Vector2& newPosition)
{
	m_position = newPosition;
}


//-----------------------------------------------------------------------------------------------
// Sets the 2D world velocity of the object
//
void Entity::SetVelocity(const Vector2& newVelocity)
{
	m_velocity = newVelocity;
}


//-----------------------------------------------------------------------------------------------
// Adds the addAmount vector to this Entity's current velocity
//
void Entity::AddToVelocity(const Vector2& addAmount)
{
	m_velocity += addAmount;

	float walkSpeed = GetWalkSpeed();
	float currSpeed = (m_velocity == Vector2::ZERO ? 0.f : m_velocity.NormalizeAndGetLength());

	currSpeed = ClampFloat(currSpeed, 0.f, walkSpeed);

	m_velocity *= currSpeed;
}


//-----------------------------------------------------------------------------------------------
// Sets the rotation angle of the object of the object
//
void Entity::SetOrientationDegrees(float newOrientationDegress)
{
	m_orientationDegrees = newOrientationDegress;
}


//-----------------------------------------------------------------------------------------------
// Sets the rotation velocity of the object (to update rotation orientation every frame)
//
void Entity::SetAngularVelocity(float newAngularVelocity)
{
	m_angularVelocity = newAngularVelocity;
}


//-----------------------------------------------------------------------------------------------
// Sets the flag for whether this entity is to be deleted this frame
//
void Entity::SetMarkedForDeath(bool newState)
{
	m_isMarkedForDeletion = newState;
	m_ageAtDeath = m_age;
}


//-----------------------------------------------------------------------------------------------
// Sets the flag for whether this entity should be checked for collisions in the physics step
//
void Entity::SetIsUsingPhysics(bool newState)
{
	m_isUsingPhysics = newState;
}


//-----------------------------------------------------------------------------------------------
// Toggles whether this entity is evaluated in the physics check
//
void Entity::ToggleIsUsingPhysics()
{
	m_isUsingPhysics = !m_isUsingPhysics;
}


//-----------------------------------------------------------------------------------------------
// Sets the Entity's map the one passed
//
void Entity::SetMap(Map* newMap)
{
	m_map = newMap;
}


//-----------------------------------------------------------------------------------------------
// Sets the current animation of this entity to the one specified by animName in its SpriteAnimSet
// If lockAnimation is true, no other animation will be played until this one is finished
//
void Entity::StartAnimation(const std::string& animName)
{
	m_spriteAnimSet->StartAnimation(animName);
}


//-----------------------------------------------------------------------------------------------
// Pauses the entity's current animation
//
void Entity::PauseAnimation()
{
	m_spriteAnimSet->PauseCurrentAnimation();
}


//-----------------------------------------------------------------------------------------------
// Plays the entity's current animation
//
void Entity::PlayAnimation()
{
	m_spriteAnimSet->PlayCurrentAnimation();
}


//-----------------------------------------------------------------------------------------------
// Sets the entity's stats to the stats passed
//
void Entity::SetStats(const Stats& newStats)
{
	for (int i = 0; i < NUM_STAT_TYPES; i++)
	{
		m_baseStats.m_stats[i] = newStats.m_stats[i];
	}
}


//-----------------------------------------------------------------------------------------------
// Checks the entity's stats and deals damage/applies knockback accordingly
//
void Entity::DoDamageCheck(Entity* attackingEntity)
{
	Stats defendingStats = GetStats();
	Stats attackingStats = attackingEntity->GetStats();

	// Apply damage
	int damage = attackingStats.m_stats[STAT_ATTACK] - defendingStats.m_stats[STAT_DEFENSE];
	if (damage < 0)
	{
		damage = 0;
	}
	
	m_health -= damage;
	if (m_health <= 0)
	{
		m_health = 0;
		SetMarkedForDeath(true);
	}
	
	// Apply the knockback to the entity's velocity
	Vector2 knockbackDirection = (m_position - attackingEntity->GetPosition()).GetNormalized();
	m_force += knockbackDirection * static_cast<float>(damage);
}


//-----------------------------------------------------------------------------------------------
// Returns name of this entity
//
std::string Entity::GetName() const
{
	return m_name;
}

//-----------------------------------------------------------------------------------------------
// Returns the faction of this entity
//
std::string Entity::GetFaction() const
{
	return m_entityDefinition->GetFaction();
}


//-----------------------------------------------------------------------------------------------
// Returns the 2D position of the object
//
Vector2 Entity::GetPosition() const
{
	return m_position;
}


//-----------------------------------------------------------------------------------------------
// Returns the 2D velocity of the object
//
Vector2 Entity::GetVelocity() const
{
	return m_velocity;
}


//-----------------------------------------------------------------------------------------------
// Returns the rotation orientation of the object, in degrees
//
float Entity::GetOrientationDegrees() const
{
	return m_orientationDegrees;
}


//-----------------------------------------------------------------------------------------------
// Returns the rotation velocity of the object
//
float Entity::GetAngularVelocity() const
{
	return m_angularVelocity;
}


//-----------------------------------------------------------------------------------------------
// Returns the physics radius of the object
//
float Entity::GetInnerRadius() const
{
	return m_entityDefinition->GetInnerRadius();
}


//-----------------------------------------------------------------------------------------------
// Returns the cosmetic radius of the object
//
AABB2 Entity::GetLocalDrawBounds() const
{
	return m_entityDefinition->GetLocalDrawBounds();
}


//-----------------------------------------------------------------------------------------------
// Returns the map that this entity is currently on
//
Map* Entity::GetMap() const
{
	return m_map;
}


//-----------------------------------------------------------------------------------------------
// Returns whether this entity is currently using physics
//
bool Entity::GetIsUsingPhysics() const
{
	return m_isUsingPhysics;
}


//-----------------------------------------------------------------------------------------------
// Returns the current health of the entity
//
int Entity::GetCurrentHealth() const
{
	return m_health;
}


//-----------------------------------------------------------------------------------------------
// Returns the max health of the entity
//
int Entity::GetMaxHealth() const
{
	return m_entityDefinition->GetMaxHealth();
}


//-----------------------------------------------------------------------------------------------
// Returns the base stats of this entity
//
Stats Entity::GetStats() const
{
	return m_baseStats;
}


//-----------------------------------------------------------------------------------------------
// Returns true if this entity can see through tiles that allow sight
//
bool Entity::CanSee() const
{
	return m_entityDefinition->CanSee();
}


//-----------------------------------------------------------------------------------------------
// Returns true if this entity can walk into tiles that allow walking
//
bool Entity::CanWalk() const
{
	return m_entityDefinition->CanWalk();
}


//-----------------------------------------------------------------------------------------------
// Returns true if this entity can fly over tiles that allow flight
//
bool Entity::CanFly() const
{
	return m_entityDefinition->CanFly();
}


//-----------------------------------------------------------------------------------------------
// Returns true if this entity can swim into tiles that allow swimming
//
bool Entity::CanSwim() const
{
	return m_entityDefinition->CanSwim();
}


//-----------------------------------------------------------------------------------------------
// Returns the view distance of this entity
//
float Entity::GetViewDistance() const
{
	return m_entityDefinition->GetViewDistance();
}


//-----------------------------------------------------------------------------------------------
// Returns the walk speed of this entity
//
float Entity::GetWalkSpeed() const
{
	return m_entityDefinition->GetWalkSpeed();
}


//-----------------------------------------------------------------------------------------------
// Returns the fly speed of this entity
//
float Entity::GetFlySpeed() const
{
	return m_entityDefinition->GetFlySpeed();
}


//-----------------------------------------------------------------------------------------------
// Returns the swim speed of this entity
//
float Entity::GetSwimSpeed() const
{
	return m_entityDefinition->GetSwimSpeed();
}


//-----------------------------------------------------------------------------------------------
// Returns the current sprite animation this entity is playing
//
SpriteAnim* Entity::GetCurrentAnimation() const
{
	return m_spriteAnimSet->GetCurrentAnimation();
}


//-----------------------------------------------------------------------------------------------
// Returns the name of the current sprite animation this entity is playing
//
std::string Entity::GetCurrentAnimationName() const
{
	return m_spriteAnimSet->GetCurrentAnimation()->GetName();
}


//-----------------------------------------------------------------------------------------------
// Returns the sprite animation set of this entity
//
SpriteAnimSet* Entity::GetSpriteAnimSet() const
{
	return m_spriteAnimSet;
}


//-----------------------------------------------------------------------------------------------
// Returns true if this entity is to be deleted this frame
//
bool Entity::IsMarkedForDeletion() const
{
	return m_isMarkedForDeletion;
}


//-----------------------------------------------------------------------------------------------
// Returns whether this entity should be checked for collisions in the physics step
//
bool Entity::IsUsingPhysics() const
{
	return m_isUsingPhysics;
}


//-----------------------------------------------------------------------------------------------
// Returns the draw bounds of this entity in world coordinates
//
AABB2 Entity::GetWorldDrawBounds() const
{
	AABB2 worldDrawBounds;
	AABB2 localDrawBounds = m_entityDefinition->GetLocalDrawBounds();

	worldDrawBounds.mins = m_position + localDrawBounds.mins;
	worldDrawBounds.maxs = m_position + localDrawBounds.maxs;

	return worldDrawBounds;
}


//-----------------------------------------------------------------------------------------------
// Generates the base stats of this actor from its definition
//
void Entity::InitializeBaseStats(const EntityDefinition* definition)
{
	for (int i = 0; i < NUM_STAT_TYPES; i++)
	{
		IntRange currStatRange = IntRange(definition->m_minBaseStats.m_stats[i], definition->m_maxBaseStats.m_stats[i]);

		m_baseStats.m_stats[i] = currStatRange.GetRandomInRange();
	}
}
