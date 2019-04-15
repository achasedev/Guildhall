/************************************************************************/
/* File: GameState_Playing.cpp
/* Author: Andrew Chase
/* Date: May 21st, 2018
/* Description: Implementation of the GameState_Playing class
/************************************************************************/
#include "Game/Entity/Player.hpp"
#include "Game/Entity/AIEntity.hpp"
#include "Game/Framework/Game.hpp"
#include "Game/Framework/World.hpp"
#include "Game/Framework/VoxelGrid.hpp"
#include "Game/Framework/VoxelFont.hpp"
#include "Game/Framework/GameCommon.hpp"
#include "Game/Framework/GameCamera.hpp"
#include "Game/Framework/GameAudioSystem.hpp"
#include "Game/Framework/CampaignManager.hpp"
#include "Game/Framework/CampaignDefinition.hpp"
#include "Game/GameStates/GameState_Playing.hpp"
#include "Game/Framework/PlayStates/PlayState.hpp"
#include "Game/Framework/PlayStates/PlayState_Rest.hpp"
#include "Game/Framework/PlayStates/PlayState_Pause.hpp"
#include "Game/Framework/PlayStates/PlayState_Stage.hpp"
#include "Game/Framework/PlayStates/PlayState_ControllerConnect.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Rendering/Core/Renderer.hpp"
#include "Engine/Rendering/Core/RenderScene.hpp"
#include "Engine/Rendering/Core/ForwardRenderingPath.hpp"
#include "Engine/Rendering/DebugRendering/DebugRenderSystem.hpp"


//-----------------------------------------------------------------------------------------------
// Base constructor
//
GameState_Playing::GameState_Playing(const std::string& campaignName)
	: GameState(0.f, 0.f)
{
	m_campaignBeingPlayed = CampaignDefinition::GetDefinitionByName(campaignName);
	GUARANTEE_OR_DIE(m_campaignBeingPlayed != nullptr, Stringf("GameState_Playing entered with missing campaign \"%s\"", campaignName.c_str()).c_str());
}


//-----------------------------------------------------------------------------------------------
// Base destructor
//
GameState_Playing::~GameState_Playing()
{
	if (m_overrideState != nullptr)
	{
		delete m_overrideState;
		m_overrideState = nullptr;
	}

	if (m_currentState != nullptr)
	{
		delete m_currentState;
		m_currentState = nullptr;
	}

	if (m_transitionState != nullptr)
	{
		delete m_transitionState;
		m_transitionState = nullptr;
	}
}


//-----------------------------------------------------------------------------------------------
// Sets up state before updating
//
bool GameState_Playing::Enter()
{
	// Player 1 started this transition, so we know their controller is connected
	Game::GetPlayers()[0] = new Player(0);

	CampaignManager* manager = Game::GetCampaignManager();
	manager->Initialize(m_campaignBeingPlayed);
	Game::GetWorld()->InititalizeForStage(manager->GetFirstStage());

	// Add the players to the world, should only ever be player 1
	Player** players = Game::GetPlayers();
	for (int i = 0; i < MAX_PLAYERS; ++i)
	{
		if (players[i] != nullptr)
		{
			Game::GetWorld()->AddEntity(players[i]);
		}
	}

	// Reset current game score
	Game::ResetScore();

	// Start the music
	Game::GetGameAudioSystem()->PlayBGM(m_campaignBeingPlayed->m_backgroundMusicTrack);

	// If we have a character select go to rest, otherwise go straight to stage
	if (m_campaignBeingPlayed->m_hasCharacterSelect)
	{
		TransitionToPlayState(new PlayState_Rest());
	}
	else
	{
		TransitionToPlayState(new PlayState_Stage());
		
		// Need to give player 1 a random player definition
		players[0]->ReinitializeWithDefinition(EntityDefinition::GetRandomPlayerDefinition());
	}

	return true;
}


//-----------------------------------------------------------------------------------------------
// Cleans up/finishes any operations before the game leaves this state
//
bool GameState_Playing::Leave()
{
	Game::GetCampaignManager()->CleanUp();
	Game::GetWorld()->CleanUp();

	// Delete the players
	Player** players = Game::GetPlayers();
	for (int i = 0; i < MAX_PLAYERS; ++i)
	{
		if (players[i] != nullptr)
		{
			delete players[i];
			players[i] = nullptr;
		}
	}

	return true;
}


//-----------------------------------------------------------------------------------------------
// Renders the enter state transition
//
void GameState_Playing::Render_Enter() const
{
	Render();
}


//-----------------------------------------------------------------------------------------------
// Begins a transition to the given state from the current state
//
void GameState_Playing::TransitionToPlayState(PlayState* state)
{
	m_transitionState = state;
	m_isTransitioning = true;

	if (m_currentState != nullptr)
	{
		m_currentState->StartLeaveTimer();
	}
	else
	{
		m_transitionState->StartEnterTimer();
	}
}


//-----------------------------------------------------------------------------------------------
// Creates a pause state to transition into, overriding any existing state
//
void GameState_Playing::PushOverrideState(PlayState* overrideState)
{
	m_overrideState = overrideState;
	m_previousTransitionStateBeforeOverride = m_isTransitioning;
	m_isTransitioning = true;
	m_overrideEntered = false;
	m_overrideState->StartEnterTimer();
}


//-----------------------------------------------------------------------------------------------
// Removes the override state, transitioning out and resuming the game
//
void GameState_Playing::PopOverrideState()
{
	m_isTransitioning = true;
	m_overrideState->StartLeaveTimer();
}


//-----------------------------------------------------------------------------------------------
// Checks for controller activity, in case one disconnects or is connected
//
void GameState_Playing::PerformControllerCheck()
{
	Player** players = Game::GetPlayers();
	InputSystem* input = InputSystem::GetInstance();

	for (int i = 0; i < MAX_PLAYERS; ++i)
	{
		if (players[i] == nullptr)
		{
			// No player exists, check if they tried to join
			XboxController& controller = input->GetController(i);

			if (controller.IsConnected() && controller.WasButtonJustPressed(XBOX_BUTTON_START))
			{
				// Player joined! Make them a normal character if it's outside the character select,
				// or a blank character otherwise
				const EntityDefinition* playerDef = nullptr;
				
				if (Game::GetCampaignManager()->GetCurrentStageNumber() == 0)
				{
					playerDef = EntityDefinition::GetDefinition("PlayerUninitialized");
				}
				else
				{
					playerDef = EntityDefinition::GetRandomPlayerDefinition();
				}

				players[i] = new Player(playerDef, i);

				Game::GetWorld()->AddEntity(players[i]);

				// Rescale the campaign manager's difficulty
				Game::RescaleDifficultyBasedOnCurrentPlayerCount();
			}
		}
		else
		{
			// Player exists, check if the controller disconnected and if so pause the game
			XboxController& controller = input->GetController(i);

			if (!controller.IsConnected())
			{
				PushOverrideState(new PlayState_ControllerConnect(i));
				return; // Don't push more than one controller connect state at once, otherwise we will override the previous
			}
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Checks for input
//
void GameState_Playing::ProcessInput()
{
	if (m_overrideState != nullptr)
	{
		m_overrideState->ProcessInput();
	}
	else if (m_currentState != nullptr)
	{
		m_currentState->ProcessInput();
	}
	else
	{
		m_transitionState->ProcessInput();
	}
}


//-----------------------------------------------------------------------------------------------
// Updates the play state
//
void GameState_Playing::Update()
{
	if (m_isTransitioning)
	{
		// Check for overrides first
		if (m_overrideState != nullptr)
		{
			if (!m_overrideEntered)
			{
				m_overrideEntered = m_overrideState->Enter();

				if (m_overrideEntered)
				{
					m_isTransitioning = false;
					m_overrideState->StartUpdating();
				}
			}
			else
			{
				bool leaveFinished = m_overrideState->Leave();

				if (leaveFinished)
				{
					m_isTransitioning = m_previousTransitionStateBeforeOverride;
					delete m_overrideState;
					m_overrideState = nullptr;
				}
			}
		}
		else if (m_currentState != nullptr) // Update on leave of the current state
		{
			bool leaveFinished = m_currentState->Leave();

			if (leaveFinished)
			{
				delete m_currentState;
				m_currentState = nullptr;

				m_transitionState->StartEnterTimer();
			}
		}
		else // Update on enter of the transition state
		{
			bool enterFinished = m_transitionState->Enter();

			if (enterFinished)
			{
				m_currentState = m_transitionState;
				m_currentState->StartUpdating();
				m_transitionState = nullptr;
				m_isTransitioning = false;
			}
		}
	}
	else
	{
		if (m_overrideState != nullptr)
		{
			m_overrideState->Update();
		}
		else
		{
			m_currentState->Update();
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Renders the gameplay state to screen
//
void GameState_Playing::Render() const
{
	if (m_isTransitioning)
	{
		if (m_overrideState != nullptr)
		{
			if (m_overrideEntered)
			{
				m_overrideState->Render_Leave();
			}
			else
			{
				m_overrideState->Render_Enter();
			}
		}
		else if (m_currentState != nullptr)
		{
			m_currentState->Render_Leave();
		}
		else
		{
			m_transitionState->Render_Enter();
		}
	}
	else if (m_overrideState != nullptr)
	{
		m_overrideState->Render();
	}
	else if (m_currentState != nullptr)
	{
		m_currentState->Render();
	}

	if (Game::AreAllPlayersInitialized())
	{
		Game::DrawScore();
	}
	else
	{
		Game::DrawHeading("CHOOSE YOUR CHARACTER", IntVector3(128, 56, 252), Vector3(0.5f, 0.f, 0.f));
	}

	if (Game::GetCampaignManager()->ShouldRenderStageInfo())
	{
		Game::DrawEnemyCountRemaining();
		Game::DrawStageNumber();
	}
}


//-----------------------------------------------------------------------------------------------
// Renders the leave state transition
//
void GameState_Playing::Render_Leave() const
{
	Render();
}
