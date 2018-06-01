/************************************************************************/
/* Project: Incursion
/* File: World.hpp
/* Author: Andrew Chase
/* Date: October 17th, 2017
/* Bugs: None
/* Description: Class to represent a collection of playable maps
/************************************************************************/
#pragma once
#include "Game/Map.hpp"

class World
{
public:
	//-----Public Methods-----

	World();											// Constructs and initializes all maps in the world
	~World();											// Deletes the maps and all entites within them, including player character

	void Update(float deltaTime);						// Updates the current map
	void Render() const;								// Renders the current map

	Map* GetCurrentMap() const;							// Returns a pointer to the map that is currently being played
	void CheckForMapChange();							// Checks to see if the player has completed the current map
	void UpdateFadeAlpha();								// Increments/decrements the alpha for the black fade texture, for level transitions
	void MoveToNextMap();								// Moves the player character to the next map, and begins updating and rendering that map
	void AddEntityToCurrentMap(Entity* entityToAdd);	// Adds the given entity to the current map
	void InitializeMapsAndEntities();					// Constructs the tile layout of all maps, and spawns all initial entities on them

	bool AreAllLevelsComplete() const;					// Returns true if the player exits the final level
	void DrawFadeScreen() const;

	void StartFadeAndChangeLevel();						// Sets the fading flag to start loading the next level

	void DrawDebugInfo() const;							// Draws the debug info for the current map


private:
	//-----Private Data-----

	const static int NUM_MAPS_IN_WORLD = 3;				// Number of maps a world contains
	int m_currentMapIndex;								// The index in the maps array of the current map
	bool m_levelsComplete;								// True if the player exited the final map
	Map* m_maps[NUM_MAPS_IN_WORLD];						// Array of playable maps

	bool m_isFading;									// True when the world is moving to a new map
	float m_textureAlphaScalar;							// Current alpha to draw the black texture at
	int m_fadeDirection;

	static const float WORLD_FADE_SPEED;				// Speed to change the alpha on the black textures
};