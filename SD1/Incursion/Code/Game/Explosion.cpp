/************************************************************************/
/* Project: Incursion
/* File: Explosion.cpp
/* Author: Andrew Chase
/* Date: October 15th, 2017
/* Bugs: None
/* Description: Implementation of the Explosion class
/************************************************************************/
#include "Game/Explosion.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Map.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"


//-----------------------------------------------------------------------------------------------
// Constructs an explosion given the position, radius and duration of the animation
//
Explosion::Explosion(Vector2 position, float radius, float duration, Map* theMap)
	: m_spriteAnimation(new SpriteAnimation(*g_explosionSpriteSheet, 0, 24, duration, PLAY_MODE_ONCE))
{
	m_position = position;
	m_orientationDegrees = GetRandomFloatInRange(0.f, 360.f);
	m_outerRadius = radius;
	m_innerRadius = 0.f;

	m_isUsingPhysics = false;
	m_faction = FACTION_NEUTRAL;
	m_entityType = ENTITY_TYPE_EXPLOSION;

	m_map = theMap;

	m_map->AddEntityToMap(this);
}


//-----------------------------------------------------------------------------------------------
// Destructor - deletes the sprite animation member
//
Explosion::~Explosion()
{
	delete m_spriteAnimation;
	m_spriteAnimation = nullptr;
}


//-----------------------------------------------------------------------------------------------
// Updates by calling the base Entity::Update() and calling update on the sprite animation
//
void Explosion::Update(float deltaTime)
{
	Entity::Update(deltaTime);

	if (!m_isMarkedForDeath)
	{
		m_spriteAnimation->Update(deltaTime);

		if (m_spriteAnimation->IsFinished())
		{
			SetMarkedForDeath(true);
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Draws the explosion using the sprite animation data member information (drawn with additive blending)
//
void Explosion::Render() const
{
	// Set up the coordinate system
	g_theRenderer->PushMatrix();
	g_theRenderer->TranslateCoordinateSystem2D(m_position.x, m_position.y);
	g_theRenderer->RotateCoordinateSystem2D(m_orientationDegrees);
	g_theRenderer->ScaleCoordinateSystem(m_outerRadius);

	// Draw with additive blending
	g_theRenderer->SetBlendMode(BLEND_MODE_ADDITIVE);

	// Gets the texture based on the tank type
	Texture explosionTexture = m_spriteAnimation->GetTexture();
	AABB2 textureCoords = m_spriteAnimation->GetCurrentTexCoords();

	// Draw the base without rotation
	g_theRenderer->DrawTexturedAABB2(AABB2::UNIT_SQUARE_CENTERED, explosionTexture, textureCoords.mins, textureCoords.maxs, Rgba::YELLOW);

	// Set the blend mode back to the default (alpha/one_minus_alpha)
	g_theRenderer->SetBlendMode(BLEND_MODE_ALPHA);

	// Reset the coordinate system
	g_theRenderer->PopMatrix();
}
