/************************************************************************/
/* File: Player.cpp
/* Author: Andrew Chase
/* Date: November 16th, 2017
/* Bugs: None
/* Description: Implementation of the Player class
/************************************************************************/
#include "Game/Player.hpp"
#include "Game/GameCommon.hpp"
#include "Game/EntityDefinition.hpp"
#include "Game/Map.hpp"
#include "Game/ProjectileDefinition.hpp"
#include "Game/Projectile.hpp"
#include "Game/Item.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Math/MathUtils.hpp"

//-----------------------------------------------------------------------------------------------
// Constructs the Player as an actor with the name "Player"
//
Player::Player(const Vector2& position, float orientation, const ActorDefinition* definition, Map* entityMap)
	: Actor(position, orientation, definition, entityMap, "Player")
{
}


//-----------------------------------------------------------------------------------------------
// Updates the player by polling for user input
//
void Player::Update(float deltaTime)
{
	// Prioritize controller input over keyboard input
	if (deltaTime > 0.f)
	{
		bool usedControllerInput = UpdateOnControllerInput(deltaTime);
		bool usedKeyboardInput = false;
		if (!usedControllerInput)
		{
			usedKeyboardInput = UpdateOnKeyboardInput(deltaTime);
		}

		// No movement input - slow the player down
		if (!usedKeyboardInput & !usedControllerInput)
		{
			float currSpeed = (m_velocity == Vector2::ZERO ? 0.f : m_velocity.NormalizeAndGetLength());
			float newSpeed = currSpeed - (GLOBAL_FRICTION * deltaTime);
			if (newSpeed < 0.2f)
			{
				newSpeed = 0.f;
			}
			m_velocity *= newSpeed;
		}
	}

	Actor::Update(deltaTime);
	m_attackCooldown -= deltaTime;
}


//-----------------------------------------------------------------------------------------------
// Draws the Player to the screen as an Actor
//
void Player::Render() const
{
	Actor::Render();
}


//-----------------------------------------------------------------------------------------------
// Restarts the player back to full health, in the same position
//
void Player::Respawn()
{
	m_health = m_entityDefinition->GetStartHealth();
	m_isMarkedForDeletion = false;
}


//-----------------------------------------------------------------------------------------------
// Checks for controller input and update accordingly
//
bool Player::UpdateOnControllerInput(float deltaTime)
{
	bool receivedControllerInput = false;

	XboxController& controller = g_theInputSystem->GetController(0);

	Vector2 stickPosition = controller.GetCorrectedStickPosition(XBOX_STICK_LEFT);
	float stickMagnitude = controller.GetCorrectedStickMagnitude(XBOX_STICK_LEFT);

	if (stickMagnitude > 0.f)
	{
		float stickOrientation = stickPosition.GetOrientationDegrees();
		m_orientationDegrees = stickOrientation;


		// Setting velocity - for now just assume walking
		float moveSpeed = m_entityDefinition->GetWalkSpeed();

		m_velocity += (Vector2::MakeDirectionAtDegrees(stickOrientation) * moveSpeed * (deltaTime * 100.f));

		// Clamp the move speed to the walk speed of the actor, for acceleration effect
		float resultingSpeed = m_velocity.NormalizeAndGetLength();
		float newSpeed = (resultingSpeed < moveSpeed ? resultingSpeed : moveSpeed);
		m_velocity *= newSpeed;

		receivedControllerInput = true;
	}

	// Firing an arrow
	if (controller.WasButtonJustPressed(XBOX_BUTTON_A))
	{
		receivedControllerInput = true;
		ShootArrow();
	}

	// Perform Melee attack
	if (controller.WasButtonJustPressed(XBOX_BUTTON_B))
	{
		receivedControllerInput = true;
		ThrustSpear();
	}
	
	return receivedControllerInput;
}


//-----------------------------------------------------------------------------------------------
// Checks for keyboard input and updates accordingly
//
bool Player::UpdateOnKeyboardInput(float deltaTime)
{
	bool receivedInput = false;

	bool upPressed		= g_theInputSystem->IsKeyPressed('W') || g_theInputSystem->IsKeyPressed(InputSystem::KEYBOARD_UP_ARROW);
	bool downPressed	= g_theInputSystem->IsKeyPressed('S') || g_theInputSystem->IsKeyPressed(InputSystem::KEYBOARD_DOWN_ARROW);
	bool leftPressed	= g_theInputSystem->IsKeyPressed('A') || g_theInputSystem->IsKeyPressed(InputSystem::KEYBOARD_LEFT_ARROW);
	bool rightPressed	= g_theInputSystem->IsKeyPressed('D') || g_theInputSystem->IsKeyPressed(InputSystem::KEYBOARD_RIGHT_ARROW);


	Vector2 inputDirection = Vector2::ZERO;

	if (upPressed)		{ inputDirection += Vector2(0.f, 1.f); }
	if (downPressed)	{ inputDirection += Vector2(0.f, -1.f); }
	if (leftPressed)	{ inputDirection += Vector2(-1.f, 0.f); }
	if (rightPressed)	{ inputDirection += Vector2(1.f, 0.f); }

	if (inputDirection != Vector2::ZERO)
	{
		m_orientationDegrees = inputDirection.GetOrientationDegrees();

		// Setting velocity - for now just assume walking
		float moveSpeed = m_entityDefinition->GetWalkSpeed();

		m_velocity += (inputDirection.GetNormalized() * moveSpeed * (deltaTime * 100.f));

		// Clamp the move speed to the walk speed of the actor, for acceleration effect
		float resultingSpeed = m_velocity.NormalizeAndGetLength();
		float newSpeed = (resultingSpeed < moveSpeed ? resultingSpeed : moveSpeed);
		m_velocity *= newSpeed;
		
		receivedInput = true;
	}

	// For shooting an arrow
	if (g_theInputSystem->WasKeyJustPressed(' '))
	{
		ShootArrow();
		receivedInput = true;
	}

	// For performing a melee attack
	if (g_theInputSystem->WasKeyJustPressed('Q'))
	{
		ThrustSpear();
		receivedInput = true;
	}

	return receivedInput;
}

//-----------------------------------------------------------------------------------------------
// Shoots an arrow in the nearest cardinal direction of the actor's forward direction
//
void Player::ShootArrow() 
{
	// Only shoot if off cooldown and the player has a bow
	if (m_attackCooldown <= 0.f && m_equippedItems[EQUIP_SLOT_BOW] != nullptr)
	{
		// Reset the timer
		m_attackCooldown = (1.f / PLAYER_ATTACKS_PER_SECOND);

		float shootDirection = GetNearestCardinalAngle(m_orientationDegrees);
		Vector2 projectileSpawnPosition = m_position + (Vector2::MakeDirectionAtDegrees(shootDirection) * GetInnerRadius());

		m_map->SpawnProjectile(projectileSpawnPosition, shootDirection, ProjectileDefinition::GetDefinitionByName("Arrow"), "PlayerArrow", GetFaction());

		// Set the animation on the actor to show shooting - based on cardinal angle
		if		(shootDirection == 0.f)		{ StartAnimation("RangedAttackEast"); }
		else if (shootDirection == 90.f)	{ StartAnimation("RangedAttackNorth"); }
		else if (shootDirection == 180.f)	{ StartAnimation("RangedAttackWest"); }
		else if (shootDirection == 270.f)	{ StartAnimation("RangedAttackSouth"); }
	}
}


//-----------------------------------------------------------------------------------------------
// Thrusts the spear in the forward direction of the player
//
void Player::ThrustSpear()
{
	// Only shoot if off cooldown and the player has a bow
	if (m_attackCooldown <= 0.f && m_equippedItems[EQUIP_SLOT_SPEAR] != nullptr)
	{
		// Reset the timer
		m_attackCooldown = (2.f / static_cast<float>(m_currentStats.m_stats[STAT_SPEED]));

		// Do melee attacks by spawning a 'fake' projectile
		float attackDirection = GetNearestCardinalAngle(m_orientationDegrees);
		Vector2 projectileSpawnPosition = m_position + (Vector2::MakeDirectionAtDegrees(attackDirection) * 0.5f);

		Projectile* meleeProjectile = m_map->SpawnProjectile(projectileSpawnPosition, attackDirection, ProjectileDefinition::GetDefinitionByName("Melee"), "", GetFaction());
		meleeProjectile->SetStats(m_equippedItems[EQUIP_SLOT_SPEAR]->GetStats());	// Set the stats to the spear's stats for damage calculation

		// Set the animation on the actor to show shooting - based on cardinal angle
		if		(attackDirection == 0.f)	{ StartAnimation("SpearAttackEast"); }
		else if (attackDirection == 90.f)	{ StartAnimation("SpearAttackNorth"); }
		else if (attackDirection == 180.f)	{ StartAnimation("SpearAttackWest"); }
		else if (attackDirection == 270.f)	{ StartAnimation("SpearAttackSouth"); }
	}
}
