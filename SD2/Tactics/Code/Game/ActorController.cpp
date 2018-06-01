/************************************************************************/
/* File: ActorController.cpp
/* Author: Andrew Chase
/* Date: March 12th, 2017
/* Description: Implementation of the ActorController class
/************************************************************************/
#include "Game/Map.hpp"
#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"
#include "Game/BoardState.hpp"
#include "Game/PlayAction.hpp"
#include "Game/PlayMode_Menu.hpp"
#include "Game/ActorController.hpp"
#include "Game/PlayMode_Observe.hpp"
#include "Game/PlayMode_Victory.hpp"
#include "Game/GameState_Playing.hpp"
#include "Engine/Renderer/OrbitCamera.hpp"


//-----------------------------------------------------------------------------------------------
// Constructor - all controllers must own a single actor for its entire lifetime
//
ActorController::ActorController(Actor& actor)
	: m_actor(actor)
{
}


//-----------------------------------------------------------------------------------------------
// Destructor
//
ActorController::~ActorController()
{
	// Sanity check - shouldn't delete (end turn) with an action still (unless done externally)
	ASSERT_OR_DIE(m_selectedAction == nullptr, "ActorController deleted with an action leftover");
}


//-----------------------------------------------------------------------------------------------
// Updates the controller by fetching a list of menu options for the actor, and after selection
// sets up and pushes the action to the GameState_Playing
//
void ActorController::Update()
{
	if (m_selectedAction != nullptr)
	{
		if (m_selectedAction->IsReady())
		{
			GameState_Playing* gameplayState = Game::GetGameStatePlaying();			
			gameplayState->EnqueueAction(m_selectedAction);
			m_selectedAction = nullptr;
		}
		else
		{
			m_selectedAction->Setup();
		}
	}
	else
	{
		// Need to setup the next action selection for the turn
		// Check for victory first here, prevents checking every frame
		bool victoryAchieved = CheckForVictory();

		if (!victoryAchieved)
		{
			// Setup the menu options and callback to the controller
			std::vector<MenuOption_t> menuOptions = Menu::MakeMenuForActor(this);

			// Push the menu mode
			GameState_Playing* gameplayState = Game::GetGameStatePlaying();
			gameplayState->PushMode(new PlayMode_Menu(this, menuOptions));
		}
	}	
}


//-----------------------------------------------------------------------------------------------
// Callback for when an action has been selected from the menu (or if selection was cancelled)
//
void ActorController::OnMenuSelection(bool wasCancelled, eActionName selectedActionName)
{
	GameState_Playing* gameplayState = Game::GetGameStatePlaying();

	// No action was selected from the menu, so go into free-roam observe mode
	if (wasCancelled)
	{
		gameplayState->PushMode(new PlayMode_Observe(this));
	}
	// Action was selected, so push a new action to the GameState_Playing
	else
	{
		m_selectedAction = PlayAction::CreateAction(this, selectedActionName);
	}
}


//-----------------------------------------------------------------------------------------------
// Sets the action class usage flag on the controller
// Used to prevent an actor from using two actions of the same class during their turn
//
void ActorController::SetControllerFlag(eActionClass actionClass, bool newState)
{
	m_controllerFlags[actionClass] = newState;
}


//-----------------------------------------------------------------------------------------------
// Checks if the given action class has been used yet this turn
//
bool ActorController::WasActionClassUsed(eActionClass actionClass) const
{
	return m_controllerFlags[actionClass];
}


//-----------------------------------------------------------------------------------------------
// Checks if the turn has ended (either the wait action was used, or the actor died)
//
bool ActorController::HasTurnEnded() const
{
	// Has the wait action been consumed?
	bool waitActionUsed = m_controllerFlags[ACTION_CLASS_WAIT];
	bool actorDied = m_actor.IsDead();

	return waitActionUsed || actorDied;
}


//-----------------------------------------------------------------------------------------------
// Returns the actor this controller possesses
//
Actor* ActorController::GetActor()
{
	return &m_actor;
}


//------------------------------------------------------------------------------------------------
// Checks if there is only one team remaining, and if so pushes a victory mode
// 
bool ActorController::CheckForVictory()
{
	BoardState* boardState = Game::GetCurrentBoardState();

	int firstTeam = boardState->GetActorByIndex(0)->GetTeamIndex();
	for (int actorIndex = 0; actorIndex < (int) boardState->GetActorCount(); actorIndex++)
	{
		Actor* currActor = boardState->GetActorByIndex(actorIndex);

		if (firstTeam != currActor->GetTeamIndex())
		{
			// Two actors on different teams, so no victory
			return false;
		}
	}

	// Every actor was on the same team, so game over
	GameState_Playing* playState = Game::GetGameStatePlaying();
	playState->PushMode(new PlayMode_Victory());

	return true;
}
