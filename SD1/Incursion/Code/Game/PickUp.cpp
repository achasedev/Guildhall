/************************************************************************/
/* Project: Incursion
/* File: PickUp.cpp
/* Author: Andrew Chase
/* Date: October 22nd, 2017
/* Bugs: None
/* Description: Implementation of the PickUp class
/************************************************************************/
#include "Game/PickUp.hpp"
#include "Game/GameCommon.hpp"
#include "Game/PlayerTank.hpp"
#include "Game/Map.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Audio/AudioSystem.hpp"

const float PickUp::PICKUP_DEFAULT_INNER_RADIUS = 0.4f;
const float PickUp::PICKUP_DEFAULT_OUTER_RADIUS = 0.5f;
const float PickUp::PICKUP_OSCILLATE_MAGNITUDE	= 0.1f;
const float PickUp::PICKUP_OSCILLATE_FREQUENCY = 200.f;


//-----------------------------------------------------------------------------------------------
// Sets up the PickUp based on its type, and automatically adds it to the map
//
PickUp::PickUp(Vector2 spawnPosition, PickUpType pickUpType, Map* theMap)
	: m_pickUpType(pickUpType)
	, m_spriteCoords(IntVector2(1, 2))	// Snowflake is the error case for now
{
	m_position = spawnPosition;
	m_anchorPosition = spawnPosition;
	m_innerRadius = PICKUP_DEFAULT_INNER_RADIUS;
	m_outerRadius = PICKUP_DEFAULT_OUTER_RADIUS;

	m_entityType = ENTITY_TYPE_PICKUP;
	m_faction = FACTION_NEUTRAL;
	m_map = theMap;
	m_map->AddEntityToMap(this);

	// Assign sprite coords by type
	switch (m_pickUpType)
	{
	case PICKUP_TYPE_HEALTH:
		m_spriteCoords = IntVector2(2, 1);
		break;
	case PICKUP_TYPE_INVINCIBILITY:
		m_spriteCoords = IntVector2(0, 0);
		break;
	case PICKUP_TYPE_REINFORCEMENTS:
		m_spriteCoords = IntVector2(3, 1);
		break;
	case PICKUP_TYPE_SPEEDBOOST:
		m_spriteCoords = IntVector2(2, 2);
		break;
	default:
		break;
	}
}


//-----------------------------------------------------------------------------------------------
// Destructor - used to play a sound when being destroyed
//
PickUp::~PickUp()
{
	// Don't play a sound if the map it's on is being destroyed
	if (m_map->IsBeingDestroyed())
	{
		return;
	}
	switch (m_pickUpType)
	{
	case PICKUP_TYPE_HEALTH:
	{
		SoundID healthSound = g_theAudioSystem->CreateOrGetSound(HEALTH_SOUND_PATH);
		g_theAudioSystem->PlaySound(healthSound);
	}
		break;
	case PICKUP_TYPE_INVINCIBILITY:
	{
		SoundID invincibilitySound = g_theAudioSystem->CreateOrGetSound(INVINCIBILITY_SOUND_PATH);
		g_theAudioSystem->PlaySound(invincibilitySound);
	}
		break;
	case PICKUP_TYPE_REINFORCEMENTS:
	{
		SoundID reinforcementsSound = g_theAudioSystem->CreateOrGetSound(REINFORCEMENTS_SOUND_PATH);
		g_theAudioSystem->PlaySound(reinforcementsSound);
	}
	break;
	case PICKUP_TYPE_SPEEDBOOST:
	{
		SoundID speedBoostSound = g_theAudioSystem->CreateOrGetSound(SPEEDBOOST_SOUND_PATH);
		g_theAudioSystem->PlaySound(speedBoostSound);
	}
	break;
	default:
		break;
	}
}


//-----------------------------------------------------------------------------------------------
// Moves the PickUp in a circle around its anchor position
//
void PickUp::Update(float deltaTime)
{
	Entity::Update(deltaTime);

	// Oscillate our position so we bob up and down
	float verticalDisplacementScalar = SinDegrees(m_age * PICKUP_OSCILLATE_FREQUENCY);
	float horizontalDisplacementScalar = CosDegrees(m_age * PICKUP_OSCILLATE_FREQUENCY);

	float verticalOffset = verticalDisplacementScalar * PICKUP_OSCILLATE_MAGNITUDE;
	float horizontalOffet = horizontalDisplacementScalar * PICKUP_OSCILLATE_MAGNITUDE;

	m_position = m_anchorPosition + Vector2(verticalOffset, horizontalOffet);
}


//-----------------------------------------------------------------------------------------------
// Draws the PickUp using the correct sprite in the sprite sheet
//
void PickUp::Render() const
{
	// Set up coordinate system
	g_theRenderer->PushMatrix();

	g_theRenderer->TranslateCoordinateSystem2D(m_position);
	g_theRenderer->RotateCoordinateSystem2D(m_orientationDegrees);
	g_theRenderer->ScaleCoordinateSystem(m_outerRadius);

	// Gets the player textures
	Texture pickUpTexture = g_pickUpSpriteSheet->GetTexture();
	AABB2 texCoordinates = g_pickUpSpriteSheet->GetTexCoordFromSpriteCoords(m_spriteCoords);

	g_theRenderer->DrawTexturedAABB2(AABB2::HALF_UNIT_SQUARE_CENTERED, pickUpTexture, texCoordinates.mins, texCoordinates.maxs, Rgba::WHITE);
	g_theRenderer->PopMatrix();
}


//-----------------------------------------------------------------------------------------------
// Applies the PickUp effect directly to the player, then marks the pickup for deletion
//
void PickUp::ApplyPickUpEffectToPlayer()
{
	switch (m_pickUpType)
	{
	case PICKUP_TYPE_HEALTH:
		// Restore the player to full health
		g_thePlayer->AddHealth(9999);
		break;
	case PICKUP_TYPE_INVINCIBILITY:
	{
		// Make the player invincible for 5 seconds, adding on to the duration if already invincible
		float playerAge = g_thePlayer->GetAge();

		if (playerAge > 3.0f)
		{
			g_thePlayer->SetAge(-2.f);
		}
		else
		{
			g_thePlayer->SetAge(playerAge - 5.0f);
		}
	}
		break;
	case PICKUP_TYPE_REINFORCEMENTS:
		// Spawn 5 allies
		for (int i = 0; i < 5; i++)
		{
			Vector2 spawnPosition = m_anchorPosition + Vector2::GetRandomVector(0.1f);
			m_map->SpawnEntity(spawnPosition, 0.f, FACTION_PLAYER, ENTITY_TYPE_NPCTANK);
		}
		break;
	case PICKUP_TYPE_SPEEDBOOST:
		// Give the player a speed boost
		g_thePlayer->ApplySpeedBoost(2.0f);
		break;
	default:
		break;
	}
	// Have the map remove this PickUp from the map
	m_isMarkedForDeath = true;
}
