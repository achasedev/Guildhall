/************************************************************************/
/* File: Game.hpp
/* Author: Andrew Chase
/* Date: September 23rd, 2017
/* Bugs: None
/* Description: Class used for managing and updating game objects and 
/*              mechanics
/************************************************************************/
#pragma once
#include <map>
#include <string>
#include <queue>
#include "Game/Dialogue.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Core/Rgba.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Math/AABB2.hpp"

class Adventure;
class AdventureDefinition;
class Map;
class Dialogue;

// Enum to represent the game's current state and transitioning state
enum GameState
{
	GAME_STATE_ERROR = -1,
	GAME_STATE_NONE,
	GAME_STATE_ATTRACT,
	GAME_STATE_PLAYING,
	GAME_STATE_PAUSED,
	GAME_STATE_DIALOGUE,
	GAME_STATE_GAMEOVER,
	GAME_STATE_VICTORY
};


class Game
{
	
public:
	//-----Public Methods-----

	Game();
	~Game();

	void Update(float deltaTime);		// Updates all game object states, called each frame
	void Render() const;				// Renders all game objects to screen, called each frame

	void StartTransitionToState(GameState nextState, bool shouldFade);
	GameState GetCurrentState() const;
	GameState GetTransitionState() const;

	void PushDialogue(const std::string& text, const AABB2& textBox=Dialogue::DEFAULT_TEXT_BOX);


private:	
	//-----Private Methods-----

	// State Update Functions
	void UpdateStateInformation(float deltaTime);

	void Update_Attract(float deltaTime);
	void Update_Playing(float deltaTime);
	void Update_Paused(float deltaTime);
	void Update_Dialogue(float deltaTime);
	void Update_GameOver(float deltaTime);
	void Update_Victory(float deltaTime);

	void UpdateBackgroundMusic(GameState prevState);

	// State Render Functions
	void RenderFade() const;
	void DrawFadeIn() const;
	void DrawFadeOut() const;

	void Render_Attract() const;
	void Render_Playing() const;
	void Render_Paused() const;
	void Render_Dialogue() const;
	void Render_GameOver() const;
	void Render_Victory() const;

	void CreateAndStartAdventure(const AdventureDefinition* adventureDefinition);

	std::string ConvertStateToString(GameState state) const;

	

private:
	//-----Private Data-----

	Adventure* m_currAdventure = nullptr;

	// State tracking
	GameState m_currentState = GAME_STATE_ATTRACT;
	float m_secondsInCurrentState = 0.f;

	GameState m_transitionToState = GAME_STATE_NONE;
	float m_secondsIntoTransition = 0.f;
	bool m_isFading = false;

	// Dialogue
	std::queue<Dialogue*> m_dialogues;

	// Audio
	SoundPlaybackID m_backgroundMusic;

	static constexpr float TRANSITION_DURATION = 0.5f;	// How many seconds transitions should take
};
