/************************************************************************/
/* File: Player.cpp
/* Author: Andrew Chase
/* Date: September 22nd, 2017
/* Description: Implementation of the player class
/************************************************************************/
#include "Game/Entity/Weapon.hpp"
#include "Game/Entity/Player.hpp"
#include "Game/Framework/Game.hpp"
#include "Game/Entity/AIEntity.hpp"
#include "Game/Framework/Game.hpp"
#include "Game/Framework/World.hpp"
#include "Game/Entity/Projectile.hpp"
#include "Game/Framework/GameCamera.hpp"
#include "Game/Animation/VoxelSprite.hpp"
#include "Game/Animation/VoxelAnimator.hpp"
#include "Game/Animation/VoxelAnimationSet.hpp"
#include "Game/Entity/Components/PhysicsComponent.hpp"

#include "Engine/Core/Window.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/Utility/StringUtils.hpp"

#include "Engine/Core/DeveloperConsole/DevConsole.hpp"

// Player colors
Rgba Player::s_playerColors[MAX_PLAYERS] = 
{
	Rgba(0, 0, 255, 255), 
	Rgba(0, 255, 0, 255),
	Rgba(128, 0, 255, 255),
	Rgba(255, 0, 128, 255)
};


//-----------------------------------------------------------------------------------------------
// Constructor
//
Player::Player(int playerID)
	: Player(EntityDefinition::GetDefinition("PlayerUninitialized"), playerID)
{
}


//-----------------------------------------------------------------------------------------------
// Constructor with a given definition
//
Player::Player(const EntityDefinition* definition, int playerID)
	: AnimatedEntity(definition)
	, m_playerID(playerID)
{
	// Assign color based on its ID
	m_color = GetColorForPlayerID(m_playerID);

	m_health = m_definition->GetInitialHealth();
	m_entityTeam = ENTITY_TEAM_PLAYER;

	m_position = Vector3(128.f, 32.f, 128.f);
}


//-----------------------------------------------------------------------------------------------
// Destructor
//
Player::~Player()
{
}


//-----------------------------------------------------------------------------------------------
// Reconstructs the player with the given definition; used for when the player selects a
// character
//
void Player::ReinitializeWithDefinition(const EntityDefinition* definition)
{
	m_definition = definition;

	// Remake the physics component
	if (m_physicsComponent != nullptr)
	{
		delete m_physicsComponent;
		m_physicsComponent = nullptr;
	}

	if (m_definition->m_physicsType == PHYSICS_TYPE_DYNAMIC)
	{
		m_physicsComponent = new PhysicsComponent(this);
	}

	// Only create a default texture if the definition has one specified
	if (m_defaultSprite != nullptr)
	{
		delete m_defaultSprite;
		m_defaultSprite = nullptr;
	}

	// Set our health
	m_health = definition->m_initialHealth;

	// Remake the animator
	delete m_animator;
	m_animator = new VoxelAnimator(m_definition->m_animationSet, m_definition->m_defaultSprite);
	m_animator->Play("idle");
}


//-----------------------------------------------------------------------------------------------
// Checks for player related input on the respective controller
//
void Player::ProcessGameplayInput()
{
	Vector3 velocity = m_physicsComponent->GetVelocity();

	XboxController& controller = InputSystem::GetInstance()->GetController(m_playerID);
	Vector2 leftStick = controller.GetCorrectedStickPosition(XBOX_STICK_LEFT);

	float currSpeed = m_physicsComponent->GetVelocity().GetLength();

	// If we have input, apply a movement force
	if (leftStick != Vector2::ZERO)
	{
		Move(leftStick);
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

	// Test damage
	if (controller.WasButtonJustPressed(XBOX_BUTTON_Y))
	{
		TakeDamage(1);
	}
}


//-----------------------------------------------------------------------------------------------
// Update
//
void Player::Update()
{
	AnimatedEntity::Update();
}


//-----------------------------------------------------------------------------------------------
// Collision callback
//
void Player::OnEntityCollision(Entity* other)
{
	AnimatedEntity::OnEntityCollision(other);
}


//-----------------------------------------------------------------------------------------------
// On damage callback
//
void Player::OnDamageTaken(int damageAmount)
{
	AnimatedEntity::OnDamageTaken(damageAmount);
}


//-----------------------------------------------------------------------------------------------
// On death callback
//
void Player::OnDeath()
{
	AnimatedEntity::OnDeath();

	Game::GetWorld()->ParticalizeEntity(this);
	Game::AddPointsToScore(PLAYER_DEATH_SCORE_PENALTY);

	Game::PlaySystemSound("Player_death");
	Game::GetGameCamera()->AddScreenShake(SCREENSHAKE_ADDED_ON_DEATH);

	// Start the respawn process
	m_respawnTimer.SetInterval(RESPAWN_INTERVAL);
}


//-----------------------------------------------------------------------------------------------
// On spawn callback
//
void Player::OnSpawn()
{
	AnimatedEntity::OnSpawn();

	if (m_currWeapon == nullptr)
	{
		EquipWeapon(new Weapon(EntityDefinition::GetDefinition("Pistol")));
	}

	Game::PlaySystemSound("Player_respawn");
}


//-----------------------------------------------------------------------------------------------
// Shoots a projectile
//
void Player::Shoot()
{
	m_currWeapon->Shoot();

	if (m_currWeapon->IsOutOfAmmo())
	{
		UnequipCurrentWeapon();
	}
}


//-----------------------------------------------------------------------------------------------
// Resets the player to be alive again after dying
//
void Player::Respawn()
{
	// Place the player in the air somewhere
	Vector3 position = Vector3(GetRandomFloatInRange(10.f, 246.f), 54.f, GetRandomFloatInRange(10.f, 246.f));
	float orientation = GetRandomFloatInRange(0.f, 360.f);

	SetPosition(position);
	SetOrientation(orientation);

	m_isMarkedForDelete = false;
	m_health = m_definition->m_initialHealth;
	
	m_physicsComponent->StopAllMovement();
	m_respawnTimer.Reset();

	OnSpawn();
}


//-----------------------------------------------------------------------------------------------
// Adds the weapon data to the player so they can shoot
//
void Player::EquipWeapon(Weapon* weapon)
{
	if (m_currWeapon != nullptr)
	{
		m_currWeapon->OnUnequip();
		delete m_currWeapon;
	}

	m_currWeapon = weapon;
	m_currWeapon->OnEquip(this);
}


//-----------------------------------------------------------------------------------------------
// Removes the current weapon from the player and deletes it
//
void Player::UnequipCurrentWeapon()
{
	// Auto equip a pistol
	EquipWeapon(new Weapon(EntityDefinition::GetDefinition("Pistol")));
}


//-----------------------------------------------------------------------------------------------
// Returns the color of this player
//
Rgba Player::GetPlayerColor() const
{
	return m_color;
}


//-----------------------------------------------------------------------------------------------
// Returns the ID of this player
//
int Player::GetPlayerID() const
{
	return m_playerID;
}


//-----------------------------------------------------------------------------------------------
// Returns the weapon currently equipped by the player
//
Weapon* Player::GetCurrentWeapon() const
{
	return m_currWeapon;
}


//-----------------------------------------------------------------------------------------------
// Returns the time left on the player's respawn timer
//
float Player::GetRespawnTimeRemaining() const
{
	return m_respawnTimer.GetTimeUntilIntervalEnds();
}


//-----------------------------------------------------------------------------------------------
// Returns whether the player is dead and currently respawning
//
bool Player::IsRespawning() const
{
	return (m_isMarkedForDelete && m_respawnTimer.GetTimeUntilIntervalEnds() > 0.f);
}


//-----------------------------------------------------------------------------------------------
// Returns the color of the player with the given id
//
Rgba Player::GetColorForPlayerID(int id)
{
	if (id == -1)
	{
		return Rgba::WHITE;
	}

	return s_playerColors[id];
}
