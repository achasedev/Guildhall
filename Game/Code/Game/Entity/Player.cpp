/************************************************************************/
/* File: Player.cpp
/* Author: Andrew Chase
/* Date: September 22nd, 2017
/* Description: Implementation of the player class
/************************************************************************/
#include "Game/Entity/Player.hpp"
#include "Game/Entity/AIEntity.hpp"
#include "Game/Framework/Game.hpp"
#include "Game/Framework/World.hpp"
#include "Game/Entity/Projectile.hpp"
#include "Game/Animation/VoxelSprite.hpp"
#include "Game/Animation/VoxelAnimator.hpp"
#include "Game/Entity/Components/PhysicsComponent.hpp"
#include "Game/Animation/VoxelAnimationSet.hpp"

#include "Engine/Core/Window.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/Utility/StringUtils.hpp"
#include "Engine/Rendering/DebugRendering/DebugRenderSystem.hpp"

#include "Engine/Core/DeveloperConsole/DevConsole.hpp"

//-----------------------------------------------------------------------------------------------
// Constructor
//
Player::Player(unsigned int playerID)
	: MovingEntity(EntityDefinition::GetDefinition("Player"))
	, m_playerID(playerID)
{
	m_isPlayer = true;
	m_health = 10;
	m_entityTeam = ENTITY_TEAM_PLAYER;
}


//-----------------------------------------------------------------------------------------------
// Destructor
//
Player::~Player()
{
}


//-----------------------------------------------------------------------------------------------
// Checks for player related input on the respective controller
//
void Player::ProcessGameplayInput()
{
	XboxController& controller = InputSystem::GetInstance()->GetController(m_playerID);
	Vector2 leftStick = controller.GetCorrectedStickPosition(XBOX_STICK_LEFT);

	float currSpeed = m_physicsComponent->GetVelocity().GetLength();

	// If we have input, apply a movement force
	if (leftStick != Vector2::ZERO)
	{
		//ApplyInputAcceleration(leftStick);
		Move(leftStick);
		m_animator->Play("walk");
	}
	else
	{
		m_animator->Play("idle");
	}

	// Orientation
	Vector2 rightStick = controller.GetCorrectedStickPosition(XBOX_STICK_RIGHT);

	if (rightStick != Vector2::ZERO)
	{
		m_orientation = rightStick.GetOrientationDegrees();
	}

	// If we have no input or are moving too fast, decelerate
	if (leftStick == Vector2::ZERO || (currSpeed > m_definition->m_maxMoveSpeed))
	{
		Decelerate();
	}

	// Test adding a force
	if (controller.WasButtonJustPressed(XBOX_BUTTON_X))
	{
		m_physicsComponent->AddForce(Vector3(leftStick.x, 0.f, leftStick.y) * -1000.f);
	}

	// Test shooting
	if (controller.GetTriggerValue(XBOX_TRIGGER_RIGHT) > 0.5f)
	{
		Shoot();
	}

	// Test Jumping
	if (controller.WasButtonJustPressed(XBOX_BUTTON_A))
	{
		Jump();
	}
}


//-----------------------------------------------------------------------------------------------
// Update
//
void Player::Update()
{
	Entity::Update();
}


//-----------------------------------------------------------------------------------------------
// Collision callback
//
void Player::OnCollision(Entity* other)
{
	Entity::OnCollision(other);

	// The player takes damage if the other is an enemy AI
// 	if (other->GetTeam() != ENTITY_TEAM_PLAYER && dynamic_cast<AIEntity*>(other) != nullptr)
// 	{
// 		TakeDamage(1);
// 		Vector3 direction = (m_position - other->GetEntityPosition()).GetNormalized();
// 		m_physicsComponent->AddImpulse(direction * 50.f);
// 	}
}


//-----------------------------------------------------------------------------------------------
// On damage callback
//
void Player::OnDamageTaken(int damageAmount)
{
	Entity::OnDamageTaken(damageAmount);
}


//-----------------------------------------------------------------------------------------------
// On death callback
//
void Player::OnDeath()
{
	Entity::OnDeath();

	Game::GetWorld()->ParticalizeEntity(this);
}


//-----------------------------------------------------------------------------------------------
// On spawn callback
//
void Player::OnSpawn()
{
	Entity::OnSpawn();

	m_animator->Play("idle", PLAYMODE_LOOP);
}


//-----------------------------------------------------------------------------------------------
// Shoots a projectile
//
void Player::Shoot()
{
	Projectile* proj = new Projectile(EntityDefinition::GetDefinition("Bullet"));
	proj->SetPosition(m_position + Vector3(0.f, 4.f, 0.f));
	proj->SetOrientation(m_orientation);
	proj->SetTeam(m_entityTeam);

	Vector2 direction = Vector2::MakeDirectionAtDegrees(m_orientation);	
	proj->GetPhysicsComponent()->SetVelocity(Vector3(direction.x, 0.f, direction.y) * 100.f);
	
	World* world = Game::GetWorld();
	world->AddEntity(proj);
}


//-----------------------------------------------------------------------------------------------
// Resets the player to be alive again after dying
//
void Player::Respawn()
{
	m_isMarkedForDelete = false;
	m_health = 10;
	
	m_physicsComponent->StopAllMovement();
}
