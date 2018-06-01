/************************************************************************/
/* File: Game.cpp
/* Author: Andrew Chase
/* Date: September 23rd, 2017
/* Bugs: None
/* Description: Implementation of the Game class, used for managing and 
/*				updating game objects and mechanics
/************************************************************************/
#include <string>
#include "Game/Game.hpp"
#include "Game/App.hpp"
#include "Game/GameCommon.hpp"
#include "Game/TileDefinition.hpp"
#include "Game/ActorDefinition.hpp"
#include "Game/ProjectileDefinition.hpp"
#include "Game/PortalDefinition.hpp"
#include "Game/AdventureDefinition.hpp"
#include "Game/Player.hpp"
#include "Game/Adventure.hpp"
#include "Game/Behavior.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Blackboard.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/FloatRange.hpp"


//-----------------------------------------------------------------------------------------------
// Default constructor, initialize any game members here
//
Game::Game()
{
	// Load XMLDefinitions

	// Entities
	ItemDefinition::LoadDefinitions();
	ProjectileDefinition::LoadDefinitions();
	Behavior::LoadAndCreatePrototypes();
	ActorDefinition::LoadDefinitions();
	PortalDefinition::LoadDefinitions();
	
	// Environment
	TileDefinition::LoadDefinitions();
	MapDefinition::LoadDefinitions();

	// Quests
	AdventureDefinition::LoadDefinitions();
	
	// Start background music
	SoundID attractID = g_theAudioSystem->CreateOrGetSound(ATTRACT_MUSIC);
	m_backgroundMusic = g_theAudioSystem->PlaySound(attractID, true, 1.0f);
}


//-----------------------------------------------------------------------------------------------
// Destructor - clean up any allocated members
//
Game::~Game()
{
	delete m_currAdventure;
	m_currAdventure = nullptr;
}


//-----------------------------------------------------------------------------------------------
// Update the movement variables of every entity in the world, as well as update the game state
// based on the input this frame
//
void Game::Update(float deltaTime)
{
	// Update state/transition before updating the rest of the game
	UpdateStateInformation(deltaTime);

	// Update the game based on state
	switch (m_currentState)
	{
	case GAME_STATE_ATTRACT:	{ Update_Attract(deltaTime);	}	break;
	case GAME_STATE_PLAYING:	{ Update_Playing(deltaTime);	}	break;
	case GAME_STATE_PAUSED:		{ Update_Paused(deltaTime);		}	break;
	case GAME_STATE_DIALOGUE:	{ Update_Dialogue(deltaTime);	}	break;
	case GAME_STATE_GAMEOVER:	{ Update_GameOver(deltaTime);	}	break;
	case GAME_STATE_VICTORY:	{ Update_Victory(deltaTime);	}	break;

	default:
		ERROR_AND_DIE(Stringf("Error: Game was in state \"%s\" in main update loop", ConvertStateToString(m_currentState).c_str()));
		break;
	}
}


//-----------------------------------------------------------------------------------------------
// Draws all entities to the screen
//
void Game::Render() const
{
	// Update the game based on state
	switch (m_currentState)
	{
	case GAME_STATE_ATTRACT:	{ Render_Attract();		}	break;
	case GAME_STATE_PLAYING:	{ Render_Playing();		}	break;
	case GAME_STATE_PAUSED:		{ Render_Paused();		}	break;
	case GAME_STATE_DIALOGUE:	{ Render_Dialogue();	}	break;
	case GAME_STATE_GAMEOVER:	{ Render_GameOver();	}	break;
	case GAME_STATE_VICTORY:	{ Render_Victory();		}	break;

	default:
		ERROR_AND_DIE(Stringf("Error: Game was in state \"%s\" in main update loop", ConvertStateToString(m_currentState).c_str()));
		break;
	}


	// Draw fade texture over it
	RenderFade();
}


//-----------------------------------------------------------------------------------------------
// Begins a transition to the nextState
//
void Game::StartTransitionToState(GameState nextState, bool shouldFade)
{
	// Only start a transition after we've fully faded-into a state
	if (!m_isFading)
	{
		if (shouldFade)
		{
			m_isFading = true;
			m_transitionToState = nextState;
		}
		else
		{
			// Don't fade, just transition immediately
			GameState prevState = m_currentState;

			m_currentState = nextState;
			m_transitionToState = GAME_STATE_NONE;

			m_secondsIntoTransition = 0.f;
			m_secondsInCurrentState = TRANSITION_DURATION/2.f;

			UpdateBackgroundMusic(prevState);
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Updates all private game members that track current and transition state
//
void Game::UpdateStateInformation(float deltaTime)
{
	// If we are transitioning, update our transition information
	if (m_transitionToState != GAME_STATE_NONE)
	{
		m_secondsIntoTransition += deltaTime;

		if (m_secondsIntoTransition >= TRANSITION_DURATION)
		{
			GameState prevState = m_currentState;

			// Done transitioning, so enter new state
			m_currentState = m_transitionToState;
			m_transitionToState = GAME_STATE_NONE;

			m_secondsIntoTransition = 0.f;
			m_secondsInCurrentState = 0.f;

			UpdateBackgroundMusic(prevState);
		}
	}
	else
	{
		// Always update the time into this state
		m_secondsInCurrentState += deltaTime;

		if (m_secondsInCurrentState >= TRANSITION_DURATION/2.f)
		{
			m_isFading = false;		// Finished fading into the next state
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Updates the game when in the attract state
//
void Game::Update_Attract(float deltaTime)
{
	// Check for input
	bool beginPressed = (g_theInputSystem->WasKeyJustPressed(' ') || g_theInputSystem->GetController(0).WasButtonJustPressed(XBOX_BUTTON_START));
	bool quitPressed = (g_theInputSystem->WasKeyJustPressed(InputSystem::KEYBOARD_ESCAPE) || g_theInputSystem->GetController(0).WasButtonJustPressed(XBOX_BUTTON_BACK));

	if (beginPressed)
	{
		const AdventureDefinition* testAdventure = AdventureDefinition::GetDefinitionByName("Marcus");
		CreateAndStartAdventure(testAdventure);
	}
	else if (quitPressed)
	{
		g_theApp->Quit();
	}

	deltaTime = 0.f; // To prevent compiler warning Todo: fix this
}


//-----------------------------------------------------------------------------------------------
// Updates the game when in the playing state
//
void Game::Update_Playing(float deltaTime)
{
	bool pausePressed = (g_theInputSystem->WasKeyJustPressed(InputSystem::KEYBOARD_ESCAPE) || g_theInputSystem->GetController(0).WasButtonJustPressed(XBOX_BUTTON_START));
	

	if (pausePressed)
	{
		StartTransitionToState(GAME_STATE_PAUSED, false);
	}
	

	// Don't update the map if the game is still fading in/out
	if (m_isFading)
	{
		deltaTime = 0.f;
	}

	if (g_theInputSystem->WasKeyJustPressed('O'))
	{
		g_thePlayer->SetMarkedForDeath(true);
	}

	if (g_theInputSystem->WasKeyJustPressed(InputSystem::KEYBOARD_F1))
	{
		g_thePlayer->ToggleIsUsingPhysics();
	}

	if (g_theInputSystem->WasKeyJustPressed('L'))
	{
		PushDialogue("Test Dialogue");
	}

	m_currAdventure->Update(deltaTime);
}


//-----------------------------------------------------------------------------------------------
// Updates the game when in the paused state
//
void Game::Update_Paused(float deltaTime)
{
	bool resumePressed = (g_theInputSystem->WasKeyJustPressed(' ') || g_theInputSystem->GetController(0).WasButtonJustPressed(XBOX_BUTTON_START));
	bool exitPressed = (g_theInputSystem->WasKeyJustPressed(InputSystem::KEYBOARD_ESCAPE) || g_theInputSystem->GetController(0).WasButtonJustPressed(XBOX_BUTTON_BACK));

	if (resumePressed)
	{
		StartTransitionToState(GAME_STATE_PLAYING, false);
	}
	else if (exitPressed)
	{
		StartTransitionToState(GAME_STATE_ATTRACT, true);
	}

	deltaTime = 0.f;
	m_currAdventure->Update(deltaTime);
}


//-----------------------------------------------------------------------------------------------
// Updates the game when in the Dialogue state
//
void Game::Update_Dialogue(float deltaTime)
{
	if (static_cast<int>(m_dialogues.size()) > 0)
	{
		// Update the current dialogue 
		m_dialogues.front()->Update(deltaTime);
		bool progressDialogue = (g_theInputSystem->GetController(0).WasButtonJustPressed(XBOX_BUTTON_A) || g_theInputSystem->WasKeyJustPressed(' '));

		if (progressDialogue)
		{
			m_dialogues.pop();
		}
	}
	else
	{
		// No dialogue left so return to playing
		StartTransitionToState(GAME_STATE_PLAYING, false);
	}

	// Still update the adventure with deltaTime 0
	deltaTime = 0.f;
	m_currAdventure->Update(deltaTime);
}


//-----------------------------------------------------------------------------------------------
// Updates the game when in the Game Over state
//
void Game::Update_GameOver(float deltaTime)
{
	bool respawnPressed = (g_theInputSystem->WasKeyJustPressed(' ') || g_theInputSystem->GetController(0).WasButtonJustPressed(XBOX_BUTTON_START));
	bool exitPressed = (g_theInputSystem->WasKeyJustPressed(InputSystem::KEYBOARD_ESCAPE) || g_theInputSystem->GetController(0).WasButtonJustPressed(XBOX_BUTTON_BACK));

	if (respawnPressed)
	{
		g_thePlayer->Respawn();
		StartTransitionToState(GAME_STATE_PLAYING, true);
	}
	else if (exitPressed)
	{
		StartTransitionToState(GAME_STATE_ATTRACT, true);
	}

	deltaTime = 0.f;
	m_currAdventure->Update(deltaTime);
}


//-----------------------------------------------------------------------------------------------
// Updates the game when in the Victory state
//
void Game::Update_Victory(float deltaTime)
{
	XboxController& controller = g_theInputSystem->GetController(0);
	
	bool returnPressed = (g_theInputSystem->WasKeyJustPressed(' ') || g_theInputSystem->WasKeyJustPressed(InputSystem::KEYBOARD_ESCAPE) || controller.WasButtonJustPressed(XBOX_BUTTON_BACK) || controller.WasButtonJustPressed(XBOX_BUTTON_START));

	if (returnPressed)
	{
		StartTransitionToState(GAME_STATE_ATTRACT, true);
	}

	deltaTime = 0.f;
	m_currAdventure->Update(deltaTime);
}


//-----------------------------------------------------------------------------------------------
// Updates the background music to fit the current state
//
void Game::UpdateBackgroundMusic(GameState prevState)
{
	switch (m_currentState)
	{
	case GAME_STATE_ATTRACT:
	{
		g_theAudioSystem->StopSound(m_backgroundMusic);
		SoundID attractID = g_theAudioSystem->CreateOrGetSound(ATTRACT_MUSIC);
		m_backgroundMusic = g_theAudioSystem->PlaySound(attractID, true, 1.0f);
	}
		break;
	case GAME_STATE_PLAYING:
	{
		// Prevent stopping and starting the same song if coming from paused
		if (prevState == GAME_STATE_PAUSED || prevState == GAME_STATE_DIALOGUE)
		{
			g_theAudioSystem->SetSoundPlaybackVolume(m_backgroundMusic, 1.0f);
		}
		else
		{
			g_theAudioSystem->StopSound(m_backgroundMusic);
			SoundID attractID = g_theAudioSystem->CreateOrGetSound(PLAYING_MUSIC);
			m_backgroundMusic = g_theAudioSystem->PlaySound(attractID, true, 1.0f);
		}
	}
		break;
	case GAME_STATE_PAUSED:
		g_theAudioSystem->SetSoundPlaybackVolume(m_backgroundMusic, 0.4f);
		break;
	case GAME_STATE_DIALOGUE:
		g_theAudioSystem->SetSoundPlaybackVolume(m_backgroundMusic, 0.4f);
		break;
	case GAME_STATE_GAMEOVER:
	{
		g_theAudioSystem->StopSound(m_backgroundMusic);
		SoundID attractID = g_theAudioSystem->CreateOrGetSound(GAMEOVER_MUSIC);
		m_backgroundMusic = g_theAudioSystem->PlaySound(attractID, true, 1.0f);
	}
		break;
	case GAME_STATE_VICTORY:
	{
		g_theAudioSystem->StopSound(m_backgroundMusic);
		SoundID attractID = g_theAudioSystem->CreateOrGetSound(VICTORY_MUSIC);
		m_backgroundMusic = g_theAudioSystem->PlaySound(attractID, true, 1.0f);
	}
		break;
	default:
		break;
	}
}


//-----------------------------------------------------------------------------------------------
// Draws a black texture over the screen, used for fading between states
//
void Game::RenderFade() const
{
	// Just entered a transition, fade out
	if (m_secondsIntoTransition > 0.f)
	{
		DrawFadeOut();
	}
	// Just entered a new state, fade in
	else if (m_secondsInCurrentState <= TRANSITION_DURATION)
	{
		DrawFadeIn();
	}
}


//-----------------------------------------------------------------------------------------------
// Renders a black fade-in texture over the screen
//
void Game::DrawFadeIn() const
{
	// Add juice - differential fade speed
	float t = (m_secondsInCurrentState / TRANSITION_DURATION);
	t = SmoothStart2(t);

	// Make the color
	unsigned char fadeAlpha = Interpolate((unsigned char)255, (unsigned char)0, t);
	Rgba fadeColor = Rgba::BLACK;
	fadeColor.a = fadeAlpha;

	// Draw the AABB2 over the screen
	g_theRenderer->SetOrtho(*g_screenBounds);
	g_theRenderer->DrawAABB2(*g_screenBounds, fadeColor);
	
}


//-----------------------------------------------------------------------------------------------
// Renders a black fade-out texture over the screen
//
void Game::DrawFadeOut() const
{
	// Add juice - differential fade speed
	float t = (m_secondsIntoTransition / TRANSITION_DURATION);
	t = SmoothStop2(t);

	// Make the color
	unsigned char fadeAlpha = Interpolate((unsigned char)0, (unsigned char)255, t);
	Rgba fadeColor = Rgba::BLACK;
	fadeColor.a = fadeAlpha;

	// Draw the AABB2 over the screen
	g_theRenderer->SetOrtho(*g_screenBounds);
	g_theRenderer->DrawAABB2(*g_screenBounds, fadeColor);
}


//-----------------------------------------------------------------------------------------------
// Renders attract-mode graphics (main menu)
//
void Game::Render_Attract() const
{
	g_theRenderer->SetOrtho(*g_screenBounds);
	g_theRenderer->DrawAABB2(*g_screenBounds, Rgba::LIGHT_BLUE);

	AABB2 textBox = AABB2(Vector2(5.f, 5.f), Vector2(11.f, 8.f));

	g_theRenderer->DrawAABB2(textBox, Rgba::BLUE);
	g_theRenderer->DrawTextInBox2D("Adventure!", textBox, Vector2(0.5f, 0.5f), 0.5f, TEXT_DRAW_WORD_WRAP);

	textBox.Translate(Vector2(0.f, -4.f));
	g_theRenderer->DrawAABB2(textBox, Rgba::BLUE);
	g_theRenderer->DrawTextInBox2D("Your Objective - Slay the bandit Marcus!", textBox, Vector2(0.5f, 0.5f), 0.3f, TEXT_DRAW_WORD_WRAP);

	g_theRenderer->DrawText2D("ATTRACT - Press SPACE or START to begin, ESC or BACK to quit", Vector2::ZERO, 0.15f);
}


//-----------------------------------------------------------------------------------------------
// Renders playing-mode graphics (map/gameplay)
//
void Game::Render_Playing() const
{
	m_currAdventure->Render();

	g_theRenderer->SetOrtho(*g_screenBounds);
	g_theRenderer->DrawText2D("PLAYING - Press ESC or START to pause", Vector2::ZERO, 0.15f);
}


//-----------------------------------------------------------------------------------------------
// Renders paused-mode graphics (map/pause overlay)
//
void Game::Render_Paused() const
{
	m_currAdventure->Render();

	g_theRenderer->SetOrtho(*g_screenBounds);
	g_theRenderer->DrawAABB2(*g_screenBounds, Rgba(0,0,0,100));
	g_theRenderer->DrawText2D("PAUSED - Press SPACE or START to resume, ESC or BACK to return to ATTRACT", Vector2::ZERO, 0.15f);
}


//-----------------------------------------------------------------------------------------------
// Renders the dialogue box at the front of the queue
//
void Game::Render_Dialogue() const
{
	// Draw the map
	m_currAdventure->Render();

	// Draw a dim overlay
	g_theRenderer->SetOrtho(*g_screenBounds);
	g_theRenderer->DrawAABB2(*g_screenBounds, Rgba(0,0,0,80));

	// Draw the dialogue at the front of the queue
	if (static_cast<int>(m_dialogues.size()) > 0)
	{
		m_dialogues.front()->Render();
	}
}


//-----------------------------------------------------------------------------------------------
// Renders gameover-mode graphics (map/game over overlay)
//
void Game::Render_GameOver() const
{
	m_currAdventure->Render();

	g_theRenderer->SetOrtho(*g_screenBounds);
	g_theRenderer->DrawAABB2(*g_screenBounds, Rgba(0,0,0,100));
	g_theRenderer->DrawText2D("GAMEOVER - Press SPACE or START to retry, ESC or BACK to return to ATTRACT", Vector2::ZERO, 0.15f);
}


//-----------------------------------------------------------------------------------------------
// Renders victory-mode graphics (victory screen)
//
void Game::Render_Victory() const
{
	m_currAdventure->Render();

	g_theRenderer->SetOrtho(*g_screenBounds);
	g_theRenderer->DrawAABB2(*g_screenBounds, Rgba(0,0,0,100));
	g_theRenderer->DrawText2D("VICTORY - Press SPACE or START or ESC or BACK to return to ATTRACT", Vector2::ZERO, 0.15f);
}


//-----------------------------------------------------------------------------------------------
// Creates an adventure from the given definition and begins play
//
void Game::CreateAndStartAdventure(const AdventureDefinition* adventureDefinition)
{
	m_currAdventure = new Adventure(adventureDefinition);
	StartTransitionToState(GAME_STATE_PLAYING, true);
}


//-----------------------------------------------------------------------------------------------
// Returns the current state of the game
//
GameState Game::GetCurrentState() const
{
	return m_currentState;
}


//-----------------------------------------------------------------------------------------------
// Returns the transition state of the game
//
GameState Game::GetTransitionState() const
{
	return m_transitionToState;
}


//-----------------------------------------------------------------------------------------------
// Creates and adds a Dialogue to the dialogue queue, and begins a transition to the dialogue
// state if not already transitioning to it
//
void Game::PushDialogue(const std::string& text, const AABB2& textBox/*=Dialogue::DEFAULT_TEXT_BOX*/)
{
	Dialogue* newDialogue = new Dialogue(text, textBox);
	m_dialogues.push(newDialogue);

	if (m_currentState != GAME_STATE_DIALOGUE)
	{
		StartTransitionToState(GAME_STATE_DIALOGUE, false);
	}
}


//-----------------------------------------------------------------------------------------------
// Receives a GameState enum and returns a string representation of that state
//
std::string Game::ConvertStateToString(GameState state) const
{
	switch (state)
	{
	case GAME_STATE_ERROR:
		return "ERROR";
		break;
	case GAME_STATE_NONE:
		return "NONE";
		break;
	case GAME_STATE_ATTRACT:
		return "ATTRACT";
		break;
	case GAME_STATE_PLAYING:
		return "PLAYING";
		break;
	case GAME_STATE_PAUSED:
		return "PAUSED";
		break;
	case GAME_STATE_DIALOGUE:
		return "Dialogue";
		break;
	case GAME_STATE_GAMEOVER:
		return "GAMEOVER";
		break;
	case GAME_STATE_VICTORY:
		return "VICTORY";
		break;
	default:
		ERROR_AND_DIE("Error: Game::ConvertStateToString received bad state input");
		break;
	}
}
