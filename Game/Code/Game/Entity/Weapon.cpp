/************************************************************************/
/* File: Weapon.cpp
/* Author: Andrew Chase
/* Date: November 24th 2018
/* Description: Implementation of the Weapon class
/************************************************************************/
#include "Game/Entity/Weapon.hpp"
#include "Game/Entity/Player.hpp"
#include "Game/Framework/Game.hpp"
#include "Game/Framework/World.hpp"
#include "Game/Entity/Projectile.hpp"
#include "Game/Framework/GameCommon.hpp"
#include "Game/Entity/Components/PhysicsComponent.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/Time/Stopwatch.hpp"

//-----------------------------------------------------------------------------------------------
// Constructor
//
Weapon::Weapon(const EntityDefinition* definition)
	: Item(definition)
	, m_shootTimer(Stopwatch(Game::GetGameClock()))
	, m_flashTimer(Stopwatch(Game::GetGameClock()))
{
	m_flashTimer.SetInterval(WEAPON_LOW_AMMO_FLASH_INTERVAL);
}


//-----------------------------------------------------------------------------------------------
// Collision Event - for getting picked up
//
void Weapon::OnEntityCollision(Entity* other)
{
	Item::OnEntityCollision(other);

	if (other->IsPlayer())
	{
		Player* player = dynamic_cast<Player*>(other);
		player->EquipWeapon(this);

		if (m_definition->m_onPickupSound != MISSING_SOUND_ID)
		{
			AudioSystem* audio = AudioSystem::GetInstance();
			audio->PlaySound(m_definition->m_onPickupSound);
		}

		Game::GetWorld()->RemoveEntity(this);
	}
}


//-----------------------------------------------------------------------------------------------
// Equips the weapon to the player
//
void Weapon::OnEquip(Entity* entityEquipping)
{
	m_entityEquippedTo = entityEquipping;

	m_shootTimer.SetInterval(1.f / m_definition->m_fireRate);
	m_currAmmoCount = m_definition->m_initialAmmoCount;
}


//-----------------------------------------------------------------------------------------------
// Callback for when the weapon is unequipped
//
void Weapon::OnUnequip()
{
}


//-----------------------------------------------------------------------------------------------
// Fires the weapon, given the weapon's fire parameters
//
void Weapon::Shoot()
{
	ASSERT_OR_DIE(m_entityEquippedTo != nullptr, "Weapon fired when it doesn't have an entity owning it");
	//ASSERT_OR_DIE(m_currAmmoCount > 0, "Weapon fired with no ammo");

	// Check the shoot timer
	if (!m_shootTimer.HasIntervalElapsed())
	{
		return;
	}

	
	// Create and fire the projectiles
	std::vector<Projectile*> shotsFired;
	int projectileCount = CreateProjectiles(shotsFired);

	World* world = Game::GetWorld();

	for (int projectileIndex = 0; projectileIndex < projectileCount; ++projectileIndex)
	{
		world->AddEntity(shotsFired[projectileIndex]);
		m_currAmmoCount--;
	}
	
	// Shoot cooldown
	m_shootTimer.SetInterval(1.f / m_definition->m_fireRate);

	// Play shoot sound
	if (m_definition->m_onShootSound != MISSING_SOUND_ID)
	{
		AudioSystem* audio = AudioSystem::GetInstance();
		audio->PlaySound(m_definition->m_onShootSound);
	}
}


//-----------------------------------------------------------------------------------------------
// Sets this weapon to have infinite ammo or not
//
void Weapon::SetHasInfiniteAmmo(bool hasInfiniteAmmo)
{
	m_hasInfiniteAmmo = hasInfiniteAmmo;
}


//-----------------------------------------------------------------------------------------------
// Returns the number of projectiles that this weapon can still spawn
//
int Weapon::GetAmmoCountRemaining() const
{
	if (m_hasInfiniteAmmo)
	{
		return 99999999;
	}

	return m_currAmmoCount;
}


//-----------------------------------------------------------------------------------------------
// Returns whether this weapon is out of ammo
//
bool Weapon::IsOutOfAmmo() const
{
	bool hasAmmoLeft = (m_currAmmoCount > 0);

	if (m_hasInfiniteAmmo || hasAmmoLeft)
	{
		return false;
	}

	return true;
}


//-----------------------------------------------------------------------------------------------
// Returns the texture of the weapon based on its current state; for UI rendering
//
const VoxelSprite* Weapon::GetTextureForUIRender()
{
	float ammoPercentage = ((float)m_currAmmoCount / (float)m_definition->m_initialAmmoCount);

	if (ammoPercentage > WEAPON_LOW_AMMO_PERCENTAGE)
	{
		return GetVoxelSprite();
	}

	// Check if we should return the texture, for flashing effect
	if (m_flashTimer.DecrementByIntervalAll() > 0)
	{
		m_showTexture = !m_showTexture;
	}

	if (m_showTexture)
	{
		return GetVoxelSprite();
	}

	return nullptr;
}



//-----------------------------------------------------------------------------------------------
// Returns true if this weapon will continuously fire if the fire button is held
//
bool Weapon::IsFullAuto() const
{
	return m_definition->m_isFullAuto;
}


//-----------------------------------------------------------------------------------------------
// Creates the projectiles from a single "shot" of this weapon
// Returns the number of projectiles created
//
int Weapon::CreateProjectiles(std::vector<Projectile*>& out_projectiles) const
{
	int projectileCountCreated = 0;
	switch (m_definition->m_spreadType)
	{
	case SPREAD_NONE:
		projectileCountCreated = CreateProjectilesForNoSpread(out_projectiles);
		break;
	case SPREAD_RANDOM:
		projectileCountCreated = CreateProjectilesForRandomSpread(out_projectiles);
		break;
	case SPREAD_FAN:
		projectileCountCreated = CreateProjectilesForFanSpread(out_projectiles);
		break;
	case SPREAD_SOURCE: 
		projectileCountCreated = CreateProjectilesForSourceSpread(out_projectiles);
		break;
	default:
		ERROR_AND_DIE("Invalid spread type");
		break;
	}

	return projectileCountCreated;
}


//-----------------------------------------------------------------------------------------------
// Creates the given number of projectiles a 2D uniform fan spread
//
int Weapon::CreateProjectilesForFanSpread(std::vector<Projectile*>& out_projectiles) const
{
	Vector3 baseDirection = m_entityEquippedTo->GetForwardVector();
	Vector3 baseSpawnPosition = m_entityEquippedTo->GetCenterPosition() + (baseDirection * WEAPON_FIRE_OFFSET);
	eEntityTeam team = m_entityEquippedTo->GetTeam();
	float projectileSpeed = m_definition->m_projectileDefinition->m_projectileSpeed;
	int countToSpawn = m_definition->m_projectilesFiredPerShot;

	float degreeOffsetPerShot = m_definition->m_fireSpread / (float) countToSpawn;
	float minDegreeOffset = ((float) (countToSpawn - 1) / 2.f) * degreeOffsetPerShot;

	// Shoot as many as we are supposed to this fire
	for (int i = 0; i < countToSpawn; ++i)
	{
		Projectile* proj = new Projectile(m_definition->m_projectileDefinition, m_entityEquippedTo->GetTeam());

		float currDegreeOffset = minDegreeOffset - (float)i * degreeOffsetPerShot;
		float currOrientation = m_entityEquippedTo->GetOrientation();
		float finalOrientation = currOrientation + currDegreeOffset;

		Vector2 finalDirection2D = Vector2::MakeDirectionAtDegrees(finalOrientation);
		Vector3 finalDirection = Vector3(finalDirection2D.x, 0.f, finalDirection2D.y);

		proj->SetPosition(baseSpawnPosition);
		proj->SetOrientation(finalOrientation);
		proj->SetTeam(team);

		proj->GetPhysicsComponent()->SetVelocity(finalDirection * projectileSpeed);

		out_projectiles.push_back(proj);
	}

	return (int) out_projectiles.size();
}


//-----------------------------------------------------------------------------------------------
// Creates the given number of projectiles with completely randomized spread
//
int Weapon::CreateProjectilesForRandomSpread(std::vector<Projectile*>& out_projectiles) const
{
	Vector3 baseDirection = m_entityEquippedTo->GetForwardVector();
	Vector3 baseSpawnPosition = m_entityEquippedTo->GetCenterPosition() + (baseDirection * WEAPON_FIRE_OFFSET);
	eEntityTeam team = m_entityEquippedTo->GetTeam();
	float projectileSpeed = m_definition->m_projectileDefinition->m_projectileSpeed;
	int countToSpawn = MinInt(m_definition->m_projectilesFiredPerShot, GetAmmoCountRemaining());
	float maxSpread = m_definition->m_fireSpread;

	// Shoot as many as we are supposed to this fire
	for (int i = 0; i < countToSpawn; ++i)
	{
		Projectile* proj = new Projectile(m_definition->m_projectileDefinition, m_entityEquippedTo->GetTeam());

		Vector3 spreadOffset;

		spreadOffset.x = GetRandomFloatInRange(-maxSpread, maxSpread);
		spreadOffset.y = GetRandomFloatInRange(-maxSpread, maxSpread);
		spreadOffset.z = GetRandomFloatInRange(-maxSpread, maxSpread);

		Vector3 finalDirection = (baseDirection + spreadOffset).GetNormalized();
		float finalOrientation = finalDirection.xz().GetOrientationDegrees();

		proj->SetPosition(baseSpawnPosition);
		proj->SetOrientation(finalOrientation);
		proj->SetTeam(team);

		proj->GetPhysicsComponent()->SetVelocity(finalDirection * projectileSpeed);

		out_projectiles.push_back(proj);
	}

	return (int) out_projectiles.size();
}


//-----------------------------------------------------------------------------------------------
// Creates the given number of projectiles with spread on the spawn position, but not on the direction
//
int Weapon::CreateProjectilesForSourceSpread(std::vector<Projectile*>& out_projectiles) const
{
	Vector3 forwardDirection = m_entityEquippedTo->GetForwardVector();
	Vector3 rightDirection = m_entityEquippedTo->GetRightVector();
	Vector3 upDirection = m_entityEquippedTo->GetUpVector();

	Vector3 baseSpawnPosition = m_entityEquippedTo->GetCenterPosition() + (forwardDirection * WEAPON_FIRE_OFFSET);
	float finalOrientation = forwardDirection.xz().GetOrientationDegrees();

	eEntityTeam team = m_entityEquippedTo->GetTeam();
	float projectileSpeed = m_definition->m_projectileDefinition->m_projectileSpeed;
	int countToSpawn = MinInt(m_definition->m_projectilesFiredPerShot, GetAmmoCountRemaining());
	float maxSpread = m_definition->m_fireSpread;

	// Shoot as many as we are supposed to this fire
	for (int i = 0; i < countToSpawn; ++i)
	{
		Projectile* proj = new Projectile(m_definition->m_projectileDefinition, m_entityEquippedTo->GetTeam());

		float upSpreadMagnitude = GetRandomFloatInRange(-maxSpread, maxSpread);
		float rightSpreadMagnitude = GetRandomFloatInRange(-maxSpread, maxSpread);
		float forwardSpreadMagnitude = GetRandomFloatInRange(-maxSpread, maxSpread);

		Vector3 spawnOffset = upDirection * upSpreadMagnitude + rightDirection * rightSpreadMagnitude + forwardDirection * 2.f * forwardSpreadMagnitude;
		Vector3 finalPosition = baseSpawnPosition + spawnOffset;

		proj->SetPosition(finalPosition);
		proj->SetOrientation(finalOrientation);
		proj->SetTeam(team);

		proj->GetPhysicsComponent()->SetVelocity(forwardDirection * projectileSpeed);

		out_projectiles.push_back(proj);
	}

	return (int) out_projectiles.size();
}


//-----------------------------------------------------------------------------------------------
// Creates the given number of projectiles with no spread, perfect accuracy
//
int Weapon::CreateProjectilesForNoSpread(std::vector<Projectile*>& out_projectiles) const
{
	Vector3 baseDirection = m_entityEquippedTo->GetForwardVector();
	Vector3 baseSpawnPosition = m_entityEquippedTo->GetCenterPosition() + (baseDirection * WEAPON_FIRE_OFFSET);
	eEntityTeam team = m_entityEquippedTo->GetTeam();
	float projectileSpeed = m_definition->m_projectileDefinition->m_projectileSpeed;
	int countToSpawn = MinInt(m_definition->m_projectilesFiredPerShot, GetAmmoCountRemaining());

	// Shoot as many as we are supposed to this fire
	for (int i = 0; i < countToSpawn; ++i)
	{
		Projectile* proj = new Projectile(m_definition->m_projectileDefinition, m_entityEquippedTo->GetTeam());

		float finalOrientation = baseDirection.xz().GetOrientationDegrees();

		proj->SetPosition(baseSpawnPosition);
		proj->SetOrientation(finalOrientation);
		proj->SetTeam(team);

		proj->GetPhysicsComponent()->SetVelocity(baseDirection * projectileSpeed);

		out_projectiles.push_back(proj);
	}

	return (int) out_projectiles.size();
}
