/************************************************************************/
/* File: Actor.cpp
/* Author: Andrew Chase
/* Date: February 22nd, 2017
/* Description: Implementation of the Actor class
/************************************************************************/
#include <string>
#include "Game/Map.hpp"
#include "Game/Game.hpp"
#include "Game/Actor.hpp"
#include "Game/Animator.hpp"
#include "Game/BoardState.hpp"
#include "Game/GameCommon.hpp"
#include "Game/PlayAction.hpp"
#include "Game/PlayAction_Move.hpp"
#include "Game/PlayAction_Wait.hpp"
#include "Game/ActorDefinition.hpp"

#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/OrbitCamera.hpp"

//-----------------------------------------------------------------------------------------------
// Constructor
//
Actor::Actor(const std::string& actorName, ActorDefinition* definition, const IntVector2& spawnCoords, int teamIndex /*=0*/)
	: m_name(actorName)
	, m_teamIndex(teamIndex)
	, m_definition(definition)
{
	// Place the actor at the correct position
	Map* map = Game::GetCurrentBoardState()->GetMap();

	m_waitTime = 100.f;	
	m_numTurnsSinceLastAction = 0;
	m_health = m_definition->GetMaxHealth();

	m_bonusBlockChance = 0.f;
	m_bonusCritChance = 0.f;

	int spawnHeight = map->GetHeightAtMapCoords(spawnCoords);
	m_worldPosition = map->MapCoordsToWorldPosition(IntVector3(spawnCoords.x, spawnCoords.y, spawnHeight));

	m_orientation = 0.f;
	m_animator = new Animator(m_definition->GetAnimationSet(), m_definition->GetDefaultSprite());
}


//-----------------------------------------------------------------------------------------------
// Destructor
//
Actor::~Actor()
{
}


//-----------------------------------------------------------------------------------------------
// Updates the actor
//
void Actor::Update()
{
}


//-----------------------------------------------------------------------------------------------
// Returns this actor's animator's current sprite
//
void Actor::RenderSprite() const
{
	// Get the game globals
	OrbitCamera* camera	= Game::GetGameCamera();

	// Get the directional sprite
	IsoSprite* isoSprite		= m_animator->GetCurrentSprite();
	Vector3 actorForward		= GetWorldForward();
	Vector3 cameraForward		= camera->GetForwardVector();
	Vector3 cameraRight			= camera->GetRightVector();
	const Sprite* spriteToDraw	= isoSprite->GetSpriteForViewAngle(actorForward, cameraForward);

	// Draw the sprite
	Renderer* renderer = Renderer::GetInstance();
	renderer->SetCurrentShaderProgram("Cutout");
	renderer->DrawSprite(spriteToDraw, m_worldPosition, Rgba::WHITE, cameraRight);
}


//-----------------------------------------------------------------------------------------------
// Returns this actor's healthbar
//
void Actor::RenderHealthBar() const
{
	// Don't draw healthbar if at full health
	if (m_health == m_definition->GetMaxHealth()) { return; }

	Renderer* renderer = Renderer::GetInstance();
	renderer->BindTexture(0, "White");
	renderer->DisableDepth();

	OrbitCamera* camera	= Game::GetGameCamera();
	Vector3 cameraRight	= camera->GetRightVector();

	Vector3 healthBarPos = m_worldPosition + Vector3(0.f, 2.f, 0.f);
	Vector2 dimensions = Vector2(1.0f, 0.2f);

	renderer->DrawAABB2_3D(healthBarPos, dimensions, AABB2::UNIT_SQUARE_OFFCENTER, cameraRight, Vector3::DIRECTION_UP, Rgba::BLACK);

	dimensions -= Vector2(0.1f, 0.05f);

	renderer->DrawAABB2_3D(healthBarPos, dimensions, AABB2::UNIT_SQUARE_OFFCENTER, cameraRight, Vector3::DIRECTION_UP, Rgba::RED);

	float fractionLeft = (float) m_health / (float) m_definition->GetMaxHealth();
	fractionLeft = ClampFloat(fractionLeft, 0.f, (float) m_definition->GetMaxHealth());

	Vector3 bottomLeftPos = healthBarPos - 0.5f * dimensions.x * cameraRight - 0.5f * dimensions.y * Vector3::DIRECTION_UP;

	Vector2 fillDimensions = Vector2(fractionLeft * dimensions.x, dimensions.y);

	renderer->DrawAABB2_3D(bottomLeftPos, fillDimensions, AABB2::UNIT_SQUARE_OFFCENTER, cameraRight, Vector3::DIRECTION_UP, Rgba::LIGHT_GREEN, Vector2(0.f, 0.f));

	renderer->EnableDepth(COMPARE_LESS, true);
}


//-----------------------------------------------------------------------------------------------
// Returns this actor's animator
//
Animator* Actor::GetAnimator() const
{
	return m_animator;
}


//-----------------------------------------------------------------------------------------------
// Returns the actor's current wait time
//
float Actor::GetWaitTime() const
{
	return m_waitTime;
}


//-----------------------------------------------------------------------------------------------
// Returns the name of this specific actor
//
std::string Actor::GetActorName() const
{
	return m_name;
}


//-----------------------------------------------------------------------------------------------
// Returns the definition's jump height
//
int Actor::GetJumpHeight() const
{
	return m_definition->GetJumpHeight();
}


//-----------------------------------------------------------------------------------------------
// Returns the definition's action speed
//
float Actor::GetActionSpeed() const
{
	return m_definition->GetActionSpeed();
}


//-----------------------------------------------------------------------------------------------
// Returns the definition's move speed
//
int Actor::GetMoveSpeed() const
{
	return m_definition->GetMoveSpeed();
}


//-----------------------------------------------------------------------------------------------
// Returns the definition's block rate plus this actor's current bonus
//
float Actor::GetBlockRate() const
{
	return m_definition->GetBlockRate() + m_bonusBlockChance;
}


//-----------------------------------------------------------------------------------------------
// Returns the definition's block rate plus this actor's current bonus
//
float Actor::GetCritChance() const
{
	return m_definition->GetCritChance() + m_bonusCritChance;
}


//-----------------------------------------------------------------------------------------------
// Returns the definition's height
//
int Actor::GetActorHeight() const
{
	return m_definition->GetActorHeight();
}


//-----------------------------------------------------------------------------------------------
// Returns the definition's strength
//
int Actor::GetStrength() const
{
	return m_definition->GetStrength();
}


//-----------------------------------------------------------------------------------------------
// Returns the definition's max health
//
int Actor::GetMaxHealth() const
{
	return m_definition->GetMaxHealth();
}


//-----------------------------------------------------------------------------------------------
// Returns this actor's team index
//
int Actor::GetTeamIndex() const
{
	return m_teamIndex;
}


//-----------------------------------------------------------------------------------------------
// Returns this actor's current orientation
//
float Actor::GetOrientation() const
{
	return m_orientation;
}


//-----------------------------------------------------------------------------------------------
// Returns this actor's current health
//
int Actor::GetCurrentHealth() const
{
	return m_health;
}


//-----------------------------------------------------------------------------------------------
// Returns this actor's current position in world space
//
Vector3 Actor::GetWorldPosition() const
{
	return m_worldPosition;
}


//-----------------------------------------------------------------------------------------------
// Returns the number of turns since this actor last acted
//
int Actor::GetTurnsSinceLastAction() const
{
	return m_numTurnsSinceLastAction;
}


//-----------------------------------------------------------------------------------------------
// Returns the name of this actor's definition
//
std::string Actor::GetDefinitionName() const
{
	return m_definition->GetName();
}

//-----------------------------------------------------------------------------------------------
// Returns the map coordinate this actor's feet is currently in
//
IntVector3 Actor::GetMapCoordinate() const
{
	IntVector3 mapPosition;

	mapPosition.x = (int) m_worldPosition.x;
	mapPosition.y = (int) m_worldPosition.z;
	mapPosition.z = (int) m_worldPosition.y;

	return mapPosition;
}


//-----------------------------------------------------------------------------------------------
// Returns this actor's forward vector in world space
//
Vector3 Actor::GetWorldForward() const
{
	Vector2 forward2D = Vector2::MakeDirectionAtDegrees(m_orientation);
	Vector3 forward3D = Vector3(forward2D.x, 0.f, forward2D.y);

	return forward3D;
}


//-----------------------------------------------------------------------------------------------
// Returns this actor's list of actions it can perform
//
const std::vector<eActionName>& Actor::GetActions() const
{
	return m_definition->GetActions();
}


//-----------------------------------------------------------------------------------------------
// Sets this actor's map position to the one given
//
void Actor::SetWorldPosition(const Vector3& position)
{
	m_worldPosition = position;
}


//-----------------------------------------------------------------------------------------------
// Sets the actor's map orientation to the one specified
//
void Actor::SetOrientation(float degrees)
{
	m_orientation = degrees;
}


//-----------------------------------------------------------------------------------------------
// Reduces this actor's health by amount, clamping between 0 and m_maxHealth
//
void Actor::TakeDamage(int damageAmount)
{
	m_health = ClampInt(m_health - damageAmount, 0, m_definition->GetMaxHealth());
}


//-----------------------------------------------------------------------------------------------
// Sets this actor's wait time to the one given
//
void Actor::SetWaitTime(float waitTime)
{
	m_waitTime = waitTime;
}


//-----------------------------------------------------------------------------------------------
// Adds the given amount to this actor's wait time
//
void Actor::AddToWaitTime(float amountToAdd)
{
	m_waitTime += amountToAdd;
}


//-----------------------------------------------------------------------------------------------
// Increments this actor's last turn count - used for tie breaks in wait time
//
void Actor::IncrementTurnCount()
{
	m_numTurnsSinceLastAction++;
}


//-----------------------------------------------------------------------------------------------
// Sets the actor's block rate to the given amount
//
void Actor::SetBlockRateBonus(float bonus)
{
	m_bonusBlockChance = bonus;
}


//-----------------------------------------------------------------------------------------------
// Increments the actor's block rate by the given amount
//
void Actor::AddToBlockRateBonus(float increment)
{
	m_bonusBlockChance += increment;
}


//-----------------------------------------------------------------------------------------------
// Sets the actor's crit chance to the given amount
//
void Actor::SetCritChanceBonus(float bonus)
{
	m_bonusCritChance = bonus;
}


//-----------------------------------------------------------------------------------------------
// Increments the actor's crit chance by the given amount
//
void Actor::AddToCritChanceBonus(float increment)
{
	m_bonusCritChance += increment;
}


//-----------------------------------------------------------------------------------------------
// Resets this actor's last turn count
//
void Actor::ClearTurnCount()
{
	m_numTurnsSinceLastAction = 0;
}


//-----------------------------------------------------------------------------------------------
// Sets the turn count to the one specified
//
void Actor::SetTurnCount(int count)
{
	m_numTurnsSinceLastAction = count;
}


//-----------------------------------------------------------------------------------------------
// Returns the number of ticks this actor will wait until their next action
//
float Actor::GetTimeUntilNextAction() const
{
	return (m_waitTime / m_definition->GetActionSpeed());
}


//-----------------------------------------------------------------------------------------------
// Returns true if the actor is dead (health == 0)
//
bool Actor::IsDead() const
{
	return (m_health <= 0);
}
