/************************************************************************/
/* File: PlayMode_Default.cpp
/* Author: Andrew Chase
/* Date: March 12th, 2018
/* Description: Class for basic turn management between actors
/************************************************************************/
#include <vector>
#include "Game/Game.hpp"
#include "Game/Actor.hpp"
#include "Game/BoardState.hpp"
#include "Game/GameCommon.hpp"
#include "Game/ActorController.hpp"
#include "Game/PlayMode_Victory.hpp"
#include "Game/PlayMode_Default.hpp"
#include "Game/GameState_Playing.hpp"


//-----------------------------------------------------------------------------------------------
// Only constructor, doesn't hold a reference to a controller since it finds it during Update()
//
PlayMode_Default::PlayMode_Default()
	: PlayMode(nullptr)
{
}


//-----------------------------------------------------------------------------------------------
// Destructor, ensure the controller is cleaned up
//
PlayMode_Default::~PlayMode_Default()
{
	if (m_actorController != nullptr)
	{
		delete m_actorController;
		m_actorController = nullptr;
	}
}


//-----------------------------------------------------------------------------------------------
// Checks for Input (should be unused)
//
void PlayMode_Default::ProcessInput()
{
}


//-----------------------------------------------------------------------------------------------
// Used to start and end turns, or update the active controller if in the middle of a turn
//
void PlayMode_Default::Update()
{
	if (m_actorController == nullptr)
	{
		StartActorTurn();
	}
	else if (m_actorController->HasTurnEnded()) // Checks for wait command used or dead actor
	{
		EndActorTurn();
	}
	else
	{
		m_actorController->Update();
	}
}


//-----------------------------------------------------------------------------------------------
// Draws all world items (unused)
//
void PlayMode_Default::RenderWorldSpace() const
{
}


//-----------------------------------------------------------------------------------------------
// Draws UI (unused)
//
void PlayMode_Default::RenderScreenSpace() const
{
}


//-----------------------------------------------------------------------------------------------
// Called before deletion (unused)
//
void PlayMode_Default::Exit()
{
}


//-----------------------------------------------------------------------------------------------
// Always returns false, ensures it is never popped from the GameState_Playing mode stack
//
bool PlayMode_Default::IsFinished() const
{
	// Default mode is never finished!
	return false;
}


//-----------------------------------------------------------------------------------------------
// Finds the next actor waiting for their turn based on wait times, and starts the turn
//
void PlayMode_Default::StartActorTurn()
{
	// Find the next actor
	AdvanceToNextReadyActor();
}


//-----------------------------------------------------------------------------------------------
// Deletes the controller, so the next actor's turn can begin
//
void PlayMode_Default::EndActorTurn()
{
	delete m_actorController;
	m_actorController = nullptr;
}


//-----------------------------------------------------------------------------------------------
// Scans through all actors and finds the one with the minimum waittime/action speed
// Ties are handled by the longest-waiting actor going first, by turn count
//
void PlayMode_Default::AdvanceToNextReadyActor()
{
	float minTicksUntilNextAction = 99999.f;
	std::vector<Actor*> actorsWithMinWaitTime;
	BoardState* boardState = Game::GetCurrentBoardState();

	// Iterate across all actors
	for (int actorIndex = 0; actorIndex < (int) boardState->GetActorCount(); actorIndex++)
	{
		Actor* currActor = boardState->GetActorByIndex(actorIndex);
		float currTimeUntilNextAction = currActor->GetTimeUntilNextAction();

		if (currTimeUntilNextAction < minTicksUntilNextAction)
		{
			minTicksUntilNextAction = currTimeUntilNextAction;
			actorsWithMinWaitTime.clear();
			actorsWithMinWaitTime.push_back(currActor);
		}
		else if (currTimeUntilNextAction == minTicksUntilNextAction)
		{
			actorsWithMinWaitTime.push_back(currActor);
		}

		// Increment the turn count on all actors, will set the count to 0 on the selected actor
		currActor->IncrementTurnCount();
	}

	// Have a list of all the actors with min wait time, now we just need to settle draws in next turn time
	int maxTurns = -1;
	Actor* longestWaitActor = nullptr;
	for (int actorIndex = 0; actorIndex < (int) actorsWithMinWaitTime.size(); actorIndex++)
	{
		// Take the actor who's been waiting the longest - this should never tie (one actor acts per turn)
		Actor* currActor = actorsWithMinWaitTime[actorIndex];
		if (currActor->GetTurnsSinceLastAction() > maxTurns)
		{
			longestWaitActor = currActor;
			maxTurns = currActor->GetTurnsSinceLastAction();
		}
	}


	// Decrement all other wait times of the other actors by this amount
	boardState->DecrementAllActorWaitTimes(minTicksUntilNextAction);
	boardState->SortActorsByNextTurn();

	// Update the current actor, resetting their turn
	longestWaitActor->ClearTurnCount();
	longestWaitActor->SetBlockRateBonus(0.f);
	longestWaitActor->SetCritChanceBonus(0.f);
	m_actorController = new ActorController(*longestWaitActor);
}
