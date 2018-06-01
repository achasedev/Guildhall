/************************************************************************/
/* File: PlayAction_Wait.cpp
/* Author: Andrew Chase
/* Date: March 12th, 2018
/* Description: Implementation of the wait PlayAction
/************************************************************************/
#include "Game/Actor.hpp"
#include "Game/GameCommon.hpp"
#include "Game/PlayAction_Wait.hpp"
#include "Game/ActorController.hpp"
#include "Engine/Math/MathUtils.hpp"


// Minimum time an actor must wait after each "wait" command
const float PlayAction_Wait::ACTOR_MIN_WAIT_TIME = 30.f;


//-----------------------------------------------------------------------------------------------
// Constructor - Takes the controller for the acting actor
//
PlayAction_Wait::PlayAction_Wait(ActorController* controller)
	: PlayAction(ACTION_CLASS_WAIT)
	, m_controller(controller)
{
}


//-----------------------------------------------------------------------------------------------
// Initializes the action by setting the wait time on the current actor
// Is flagged as ready so it can be pushed and then removed from the game's action queue
//
void PlayAction_Wait::Setup()
{
	// Set the min wait time for the current actor
	Actor* actor = m_controller->GetActor();
	actor->SetWaitTime(MaxFloat(actor->GetWaitTime(), ACTOR_MIN_WAIT_TIME));
	m_actionState = ACTION_STATE_READY;
}


//-----------------------------------------------------------------------------------------------
// Called when the action first begins updating
//
void PlayAction_Wait::Start()
{
	m_actionState = ACTION_STATE_RUNNING;
}


//-----------------------------------------------------------------------------------------------
// Update - just marks it as finished so it can be deleted
//
void PlayAction_Wait::Update()
{
	m_actionState = ACTION_STATE_FINISHED;
}


//-----------------------------------------------------------------------------------------------
// Render world space elements
//
void PlayAction_Wait::RenderWorldSpace() const
{
}


//-----------------------------------------------------------------------------------------------
// Render screen elements
//
void PlayAction_Wait::RenderScreenSpace() const
{

}


//-----------------------------------------------------------------------------------------------
// Called before deletion of this action
//
void PlayAction_Wait::Exit()
{
	if (m_actionState == ACTION_STATE_FINISHED)
	{
		m_controller->SetControllerFlag(ACTION_CLASS_WAIT, true);
	}
}
