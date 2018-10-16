/************************************************************************/
/* File: Player.cpp
/* Author: Andrew Chase
/* Date: September 22nd, 2017
/* Description: Implementation of the player class
/************************************************************************/
#include "Game/Entity/Player.hpp"
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
void Player::ProcessInput()
{
	//UpdateMovementParamsOnInput();
	//DebugRenderMovementParams();

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

	// If we have no input or are moving too fast, decelerate
	if (leftStick == Vector2::ZERO || (currSpeed > m_definition->m_maxMoveSpeed))
	{
		Decelerate();
		//ApplyDeceleration();
	}

	// Test adding a force
	if (controller.WasButtonJustPressed(XBOX_BUTTON_X))
	{
		m_physicsComponent->AddForce(Vector3(leftStick.x, 0.f, leftStick.y) * -1000.f);
	}

	// Test shooting
	if (controller.IsButtonPressed(XBOX_BUTTON_B))
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

	Vector2 direction = Vector2::MakeDirectionAtDegrees(m_orientation);	
	proj->GetPhysicsComponent()->SetVelocity(Vector3(direction.x, 0.f, direction.y) * 100.f);
	
	World* world = Game::GetWorld();
	world->AddEntity(proj);
}
