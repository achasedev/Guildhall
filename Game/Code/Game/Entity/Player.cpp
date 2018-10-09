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
	: MovingEntity(EntityDefinition::GetDefinition("Robot"))
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
	Projectile* proj = new Projectile(m_definition);
	proj->SetPosition(m_position + Vector3(0.f, 4.f, 0.f));
	proj->SetOrientation(m_orientation);

	Vector2 direction = Vector2::MakeDirectionAtDegrees(m_orientation);	
	proj->GetPhysicsComponent()->SetVelocity(Vector3(direction.x, 0.f, direction.y) * 100.f);
	
	World* world = Game::GetWorld();
	world->AddEntity(proj);
}


// -----------------------------------------------------------------------------------------------
// Checks for input to update the player's base movement parameters
// 
// void Player::UpdateMovementParamsOnInput()
// {
// 	InputSystem* input = InputSystem::GetInstance();
// 	float deltaTime = Game::GetDeltaTime();
// 
// 	float maxAccelChange = 0.f;
// 	if (input->IsKeyPressed('T'))
// 	{
// 		maxAccelChange -= 20.f * deltaTime;
// 	}
// 
// 	if (input->IsKeyPressed('Y'))
// 	{
// 		maxAccelChange += 20.f * deltaTime;
// 	}
// 
// 	m_maxMoveAcceleration += maxAccelChange;
// 
// 	float maxDecelChange = 0.f;
// 	if (input->IsKeyPressed('O'))
// 	{
// 		maxDecelChange -= 20.f * deltaTime;
// 	}
// 
// 	if (input->IsKeyPressed('P'))
// 	{
// 		maxDecelChange += 20.f * deltaTime;
// 	}
// 
// 	m_maxMoveDeceleration += maxDecelChange;
// 
// 	float maxSpeedChange = 0.f;
// 	if (input->IsKeyPressed('U'))
// 	{
// 		maxSpeedChange -= 20.f * deltaTime;
// 	}
// 
// 	if (input->IsKeyPressed('I'))
// 	{
// 		maxSpeedChange += 20.f * deltaTime;
// 	}
// 
// 	m_maxMoveSpeed += maxSpeedChange;
// }
// 
// 
// -----------------------------------------------------------------------------------------------
// Renders the player's current movement params and hotkeys for changing
// 
// void Player::DebugRenderMovementParams()
// {
// 	AABB2 bounds = Window::GetInstance()->GetWindowBounds();
// 	std::string toPrint = Stringf("(T,Y) Max Acceleration: %.2f", m_maxMoveAcceleration);
// 	toPrint += Stringf("\n(U,I) Max Deceleration: %.2f", m_maxMoveDeceleration);
// 	toPrint += Stringf("\n(O,P) Max Speed: %.2f", m_maxMoveSpeed);
// 
// 	DebugRenderSystem::Draw2DText(toPrint, bounds, 0.f, Rgba::WHITE, 30.f);
// }
// 
// 
// -----------------------------------------------------------------------------------------------
// Applies the acceleration from the player's input to move them
// 
// void Player::ApplyInputAcceleration(const Vector2& inputDirection)
// {
// 	float currLateralSpeed = m_physicsComponent->GetVelocity().xz().GetLength();
// 	float deltaTime = Game::GetDeltaTime();
// 
// 	Vector2 maxLateralVelocity = (m_physicsComponent->GetVelocity().xz() + (m_maxMoveAcceleration * deltaTime) * inputDirection);
// 	float maxLateralSpeed = maxLateralVelocity.NormalizeAndGetLength();
// 
// 	maxLateralSpeed = (currLateralSpeed > m_maxMoveSpeed ? ClampFloat(maxLateralSpeed, 0.f, currLateralSpeed) : ClampFloat(maxLateralSpeed, 0.f, m_maxMoveSpeed));
// 	maxLateralVelocity *= maxLateralSpeed;
// 
// 	Vector3 inputVelocityResult = Vector3(maxLateralVelocity.x, m_physicsComponent->GetVelocity().y, maxLateralVelocity.y) - m_physicsComponent->GetVelocity();
// 	Vector3 acceleration = inputVelocityResult / deltaTime;
// 	Vector3 force = acceleration * m_mass;
// 
// 	m_physicsComponent->AddForce(force);
// 
// 	// Reorient the player
// 	m_orientation = inputDirection.GetOrientationDegrees();
// }
// 
// 
// -----------------------------------------------------------------------------------------------
// Applies a deceleration to the player so they slow down
// 
// void Player::ApplyDeceleration()
// {
// 	float deltaTime = Game::GetDeltaTime();
// 	float currSpeed = m_physicsComponent->GetVelocity().xz().GetLength();
// 	float amountCanBeDecreased = currSpeed;
// 
// 	if (amountCanBeDecreased > 0.f)
// 	{
// 		Vector2 direction = -1.0f * m_physicsComponent->GetVelocity().xz().GetNormalized();
// 
// 		float decelMag = amountCanBeDecreased / deltaTime;
// 		decelMag = ClampFloat(decelMag, 0.f, m_maxMoveDeceleration);
// 
// 		float forceMag = decelMag * m_mass;
// 
// 		direction *= forceMag;
// 		Vector3 finalForce = Vector3(direction.x, 0.f, direction.y);
// 
// 		m_physicsComponent->AddForce(finalForce);
// 	}
// }
