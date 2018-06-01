/************************************************************************/
/* Project: Incursion
/* File: World.cpp
/* Author: Andrew Chase
/* Date: October 17th, 2017
/* Bugs: None
/* Description: Implementation of the World class
/************************************************************************/
#include "Game/World.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
#include "Game/PlayerTank.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"

const float World::WORLD_FADE_SPEED = 0.05f;

//-----------------------------------------------------------------------------------------------
// Constructor - constructs all maps, but does not initialize them with tiles or entities
//
World::World()
	: m_currentMapIndex(0)
	, m_levelsComplete(false)
	, m_textureAlphaScalar(1.f)
	, m_fadeDirection(-1)
	, m_isFading(true)
{
	for (int mapIndex = 0; mapIndex < NUM_MAPS_IN_WORLD; mapIndex++)
	{
		m_maps[mapIndex] = new Map(mapIndex);
	}
}


//-----------------------------------------------------------------------------------------------
// Deletes all maps and all entities on them, including the player character
//
World::~World()
{
	for (int mapIndex = 0; mapIndex < NUM_MAPS_IN_WORLD; mapIndex++)
	{
		delete m_maps[mapIndex];
		m_maps[mapIndex] = nullptr;
	}
}


//-----------------------------------------------------------------------------------------------
// Updates the current map, and checks to see if the game should progress to the next map
//
void World::Update(float deltaTime)
{
	m_maps[m_currentMapIndex]->Update(deltaTime);

	CheckForMapChange();

	if (m_isFading)
	{
		UpdateFadeAlpha();
	}
}


//-----------------------------------------------------------------------------------------------
// Draws the current map to screen
//
void World::Render() const
{
	m_maps[m_currentMapIndex]->Render();

	// Draw debug info here before messing with setting glOrtho
	if (g_theGame->IsDrawingDebugInfo())
	{
		DrawDebugInfo();
	}

	// Draw the fade texture with the correct alpha
	if (m_isFading)
	{
		DrawFadeScreen();
	}
}


//-----------------------------------------------------------------------------------------------
// Returns the pointer to the current map
//
Map* World::GetCurrentMap() const
{
	return m_maps[m_currentMapIndex];
}


//-----------------------------------------------------------------------------------------------
// Checks if the player touched the exit tile of the current map, and if so moves them to the
// next map
//
void World::CheckForMapChange()
{
	if (m_maps[m_currentMapIndex]->DidPlayerFinishLevel())
	{
		if (!m_isFading)
		{
			m_isFading = true;

			// Play the map change sound effect
			SoundID mapChangeSound = g_theAudioSystem->CreateOrGetSound(MAP_CHANGE_SOUND_PATH);
			g_theAudioSystem->PlaySound(mapChangeSound);
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Updates the black texture alpha based on whether or not the world is transitioning to a new map
//
void World::UpdateFadeAlpha()
{
	if (m_isFading)
	{
		m_textureAlphaScalar += static_cast<float>(m_fadeDirection) * WORLD_FADE_SPEED;
		m_textureAlphaScalar = ClampFloatZeroToOne(m_textureAlphaScalar);

		// If we hit either full alpha or zero alpha, stop fading and flip the direction for
		// the next fade
		if (m_textureAlphaScalar == 1.f || m_textureAlphaScalar == 0.f)
		{
			m_fadeDirection *= -1;
			m_isFading = false;

			// If we've completely faded out, move to the next map
			if (m_textureAlphaScalar == 1.f)
			{
				MoveToNextMap();
			}
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Moves the player character to the next map if there is one, and sets the next map as the
// current map
//
void World::MoveToNextMap()
{
	if (m_currentMapIndex < (NUM_MAPS_IN_WORLD - 1))
	{
		m_maps[m_currentMapIndex]->RemovePlayerFromMap();
		m_currentMapIndex++;
		m_maps[m_currentMapIndex]->AddEntityToMap(g_thePlayer);
		
		g_thePlayer->SetAge(0.f);	// To make the player invincible at map load
		m_isFading = true;			// Fade in
	}
	else
	{
		m_levelsComplete = true;
	}
}


//-----------------------------------------------------------------------------------------------
// Adds the given entity to the current map's lists of entities
//
void World::AddEntityToCurrentMap(Entity* entityToAdd)
{
	m_maps[m_currentMapIndex]->AddEntityToMap(entityToAdd);
}


//-----------------------------------------------------------------------------------------------
// Sets up the initial state of the maps by generating the tile layout and spawning initial
// entities on them
//
void World::InitializeMapsAndEntities()
{
	for (int mapIndex = 0; mapIndex < NUM_MAPS_IN_WORLD; mapIndex++)
	{
		m_maps[mapIndex]->SetupInitialState();
	}
}


//-----------------------------------------------------------------------------------------------
// Returns true if the player exited the final level
//
bool World::AreAllLevelsComplete() const
{
	return m_levelsComplete;
}


//-----------------------------------------------------------------------------------------------
// Draws a black AABB2 over the entire screen, for fading between levels
//
void World::DrawFadeScreen() const
{
	// Drawing parameters
	AABB2 screenBounds = AABB2(Vector2(0.f, 0.f), Vector2(16.f, 9.f));
	g_theRenderer->SetOrtho(screenBounds);

	// Create the alpha color
	Rgba drawColor = Rgba::BLACK;
	drawColor.ScaleAlpha(m_textureAlphaScalar);

	// Draw the AABB2
	g_theRenderer->DrawAABB2(screenBounds, drawColor);
}


//-----------------------------------------------------------------------------------------------
// Sets the fading flag to start loading the next level
//
void World::StartFadeAndChangeLevel()
{
	m_isFading = true;
}


//-----------------------------------------------------------------------------------------------
// Draws the debug info on the current map
//
void World::DrawDebugInfo() const
{
	m_maps[m_currentMapIndex]->DrawDebugInfo();
}

